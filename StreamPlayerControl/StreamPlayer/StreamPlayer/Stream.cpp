#include "stream.h"
#include <stdexcept>

#include "frame.h"
#include "StreamPlayer.h"

using namespace std;
using namespace boost;
using namespace FFmpeg;
using namespace FFmpeg::Facade;

Stream::Stream(string const& streamUrl,	int32_t connectionTimeoutInMilliseconds,
	int32_t frameTimeoutInMilliseconds, RtspTransport transport, RtspFlags flags)
    : url_(streamUrl), connectionTimeout_(connectionTimeoutInMilliseconds),
    frameTimeout_(frameTimeoutInMilliseconds), transport_(transport), flags_(flags),
    connectionStart_((std::chrono::time_point<std::chrono::system_clock>::max)()),
    frameStart_((std::chrono::time_point<std::chrono::system_clock>::max)()),
	barrier_(2), stopRequested_(false), videoStreamIndex_(-1),	
	videoClock_(0.0), videoTimer_(0.0),	lastFrameTimestamp_(0.0), lastFrameDelay_(0.0)
{
}

void FFmpeg::Facade::Stream::WaitForOpen()
{
	workerThread_ = thread(&Stream::OpenAndRead, this);

    barrier_.wait();

	if (!IsOpen())
	{
		throw std::runtime_error(error_);
	}
}

int Stream::InterruptCallback(void *ctx)
{
    Stream* streamPtr = reinterpret_cast<Stream*>(ctx);    
    if (streamPtr == nullptr)
    {
        return 0;
    }

	if (!streamPtr->IsOpen())
	{
		if (IsTimedOut(streamPtr->connectionStart_, streamPtr->connectionTimeout_))
		{
			return 1;
		}
	}

	if (streamPtr->stopRequested_)
	{
		return 1;
	}

	if (IsTimedOut(streamPtr->frameStart_, streamPtr->frameTimeout_))
	{
		return 1;
	}

    return 0;
}

bool Stream::IsTimedOut(std::chrono::time_point<std::chrono::system_clock> start,
	std::chrono::milliseconds timeout)
{
	auto now = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
	return elapsed > timeout;
}

unique_ptr<AVDictionary, std::function<void(AVDictionary*)>> Stream::GetOptions(RtspTransport transport,
	RtspFlags flags)
{
	AVDictionary *optionsPtr = nullptr;
	if (transport == RtspTransport::Http)
	{
		av_dict_set(&optionsPtr, "rtsp_transport", "http", 0);
	}
	else if (transport == RtspTransport::Tcp)
	{
		av_dict_set(&optionsPtr, "rtsp_transport", "tcp", 0);
	}
	else if (transport == RtspTransport::Udp)
	{
		av_dict_set(&optionsPtr, "rtsp_transport", "udp", 0);
	}
	else if (transport == RtspTransport::UdpMulticast)
	{
		av_dict_set(&optionsPtr, "rtsp_transport", "udp_multicast", 0);
	}

	if (flags == RtspFlags::FilterSrc)
	{
		av_dict_set(&optionsPtr, "rtsp_flags", "filter_src", 0);
	}
	else if (flags == RtspFlags::Listen)
	{
		av_dict_set(&optionsPtr, "rtsp_flags", "listen", 0);
	}
	else if (flags == RtspFlags::PreferTcp)
	{
		av_dict_set(&optionsPtr, "rtsp_flags", "prefer_tcp", 0);
	}

	unique_ptr<AVDictionary, std::function<void(AVDictionary*)>>
		options(optionsPtr, [](AVDictionary* ptr)
	{
		av_dict_free(&ptr);
	});

	return options;
}

void Stream::Open()
{
    static once_flag flag = BOOST_ONCE_INIT;
    call_once(flag, []()
    {
        av_register_all();
        avdevice_register_all();
        avcodec_register_all();
        avformat_network_init();
    });

	AVFormatContext *formatContextPtr = avformat_alloc_context();

	formatContextPtr->interrupt_callback.callback = InterruptCallback;
	formatContextPtr->interrupt_callback.opaque = this;
	formatContextPtr->flags |= AVFMT_FLAG_NONBLOCK;

    connectionStart_ = std::chrono::system_clock::now();

	auto options = GetOptions(transport_, flags_);
	auto optionsPtr = options.release();

    int error = avformat_open_input(&formatContextPtr, url_.c_str(), nullptr, &optionsPtr);
	options.reset(optionsPtr);
    if (error != 0)
    {
		avformat_free_context(formatContextPtr);
        throw runtime_error("avformat_open_input() failed: " + AvStrError(error));
    }

	unique_ptr<AVFormatContext, std::function<void(AVFormatContext*)>>
		formatContext(formatContextPtr, [](AVFormatContext* ptr)
	{
		avformat_close_input(&ptr);
		avformat_free_context(ptr);
	});

    error = avformat_find_stream_info(formatContext.get(), nullptr);
    if (error < 0)
    {
        throw runtime_error("avformat_find_stream_info() failed: " + AvStrError(error));
    }

	AVStream *videoStreamPtr = nullptr;
    for (uint32_t i = 0; i < formatContext->nb_streams; i++)
    {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex_ = i;
			videoStreamPtr = formatContext->streams[i];
            break;
        }
    }

    if (videoStreamIndex_ == -1 || videoStreamPtr == nullptr)
    {
        throw runtime_error("no video stream");
    }

    AVCodec *codecPtr = avcodec_find_decoder(videoStreamPtr->codecpar->codec_id);
    if (codecPtr == nullptr)
    {
        throw runtime_error("avcodec_find_decoder() failed");
    }

	unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>>
		codecContext(avcodec_alloc_context3(codecPtr), [](AVCodecContext* ptr)
	{
		avcodec_free_context(&ptr);
	});

	error = avcodec_parameters_to_context(codecContext.get(), videoStreamPtr->codecpar);
	if (error < 0)
	{
		throw runtime_error("avcodec_parameters_to_context() failed: " + AvStrError(error));
	}	

	options = GetOptions(transport_, flags_);
	optionsPtr = options.release();

    error = avcodec_open2(codecContext.get(), codecPtr, &optionsPtr);
	options.reset(optionsPtr);
    if (error < 0)
    {
        throw runtime_error("avcodec_open2() failed: " + AvStrError(error));
    }

	videoTimer_ = static_cast<double>(av_gettime() / 1000000.0);
	lastFrameDelay_ = 40e-3;

	formatContext_.swap(formatContext);
	codecContext_.swap(codecContext);
}

void Stream::Read()
{
	const int32_t maxPacketQueueSize = 256;
	const int32_t readerSleepTimeInMilliseconds = 10;

	for (;;)
    {
		if (packetQueue_.Size() >= maxPacketQueueSize)
		{
			boost::this_thread::sleep_for(boost::chrono::milliseconds(readerSleepTimeInMilliseconds));
			continue;
		}

		unique_ptr<AVPacket, std::function<void(AVPacket*)>>
			packetPtr(av_packet_alloc(), [](AVPacket* ptr)
		{
			av_packet_free(&ptr);
		});

		frameStart_ = std::chrono::system_clock::now();
        int error = av_read_frame(formatContext_.get(), packetPtr.get());
        if (error < 0)
        {
            //if (error != static_cast<int>(AVERROR_EOF))
            //{
            //    throw runtime_error("av_read_frame() failed: " + AvStrError(error));
            //}

            packetQueue_.Push(nullptr);

            // The end of a stream.
            break;
        }

        if (packetPtr->stream_index == videoStreamIndex_)
        {
            packetQueue_.Push(packetPtr.release());
        }
    }
}

void Stream::OpenAndRead()
{
    try
    {
        Open();
    }
    catch (runtime_error &e)
    {
        error_ = e.what();
    }

    barrier_.wait();

    if (IsOpen())
    {
		Read();
    }    
}

bool FFmpeg::Facade::Stream::IsOpen() const
{
	return formatContext_ != nullptr && codecContext_ != nullptr && videoStreamIndex_ > -1;
}

double Stream::GetTimestamp(AVFrame *avframePtr, const AVRational& timeBase)
{
	double timestamp = 0.0;
	if (avframePtr->pkt_dts != AV_NOPTS_VALUE)
	{
		timestamp = avframePtr->best_effort_timestamp * av_q2d(timeBase);
	}

	if (timestamp != 0.0)
	{
		/* if we have pts, set video clock to it */
		videoClock_ = timestamp;
	}
	else
	{
		/* if we aren't given a pts, set it to the clock */
		timestamp = videoClock_;
	}
	/* update the video clock */
	double frameDelay = av_q2d(timeBase);
	/* if we are repeating a frame, adjust clock accordingly */
	frameDelay += avframePtr->repeat_pict * (frameDelay * 0.5);
	videoClock_ += frameDelay;

	return timestamp;
}

unique_ptr<Frame> Stream::CreateFrame(AVFrame *avframePtr)
{
	AVPixelFormat pixelFormat = AV_PIX_FMT_BGR24;

	unique_ptr<AVFrame, std::function<void(AVFrame*)>>
		avRgbFramePtr(av_frame_alloc(), [](AVFrame* ptr)
	{
		av_freep(&ptr->data[0]);
		av_frame_free(&ptr);
	});

	av_image_alloc(avRgbFramePtr->data, avRgbFramePtr->linesize,
		codecContext_->width, codecContext_->height, pixelFormat, 1);

	if (imageConvertContext_ == nullptr)
	{
		auto rawPtr = sws_getContext(codecContext_->width, codecContext_->height,
			codecContext_->pix_fmt, codecContext_->width, codecContext_->height,
			pixelFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);

		unique_ptr<SwsContext, std::function<void(SwsContext*)>>
			imageConvertCtxPtr(rawPtr, [](SwsContext* ptr)
		{
			sws_freeContext(ptr);
		});

		if (imageConvertCtxPtr == nullptr)
		{
			throw runtime_error("sws_getContext() failed");
		}

		imageConvertContext_.swap(imageConvertCtxPtr);
	}

	sws_scale(imageConvertContext_.get(), avframePtr->data,
		avframePtr->linesize, 0, codecContext_->height,
		avRgbFramePtr->data, avRgbFramePtr->linesize);

	double timestamp = GetTimestamp(avframePtr, 
		formatContext_->streams[videoStreamIndex_]->time_base);

	unique_ptr<Frame> framePtr = make_unique<Frame>(codecContext_->width,
		codecContext_->height, timestamp, *avRgbFramePtr);

	return framePtr;
}

unique_ptr<Frame> Stream::GetNextFrame()
{
    unique_ptr<Frame> framePtr;    

	for (;;)
    {
        AVPacket *rawPtr = nullptr;
        if (!packetQueue_.WaitAndPop(rawPtr))
        {			
            break;
        }

		unique_ptr<AVPacket, std::function<void(AVPacket*)>>
			packetPtr(rawPtr, [](AVPacket* ptr)
		{
			av_packet_free(&ptr);
		});

		int error = avcodec_send_packet(codecContext_.get(), packetPtr.get());
        if (error == AVERROR_EOF)
		{
			break;
		}
		else if (error < 0 && error != AVERROR(EAGAIN))
		{
			throw runtime_error("avcodec_send_packet() failed: " + AvStrError(error));
		}

		unique_ptr<AVFrame, std::function<void(AVFrame*)>>
			avframePtr(av_frame_alloc(), [](AVFrame* ptr)
		{
			av_frame_free(&ptr);
		});
		error = avcodec_receive_frame(codecContext_.get(), avframePtr.get());
		if (error == 0)
		{
			framePtr = CreateFrame(avframePtr.get());
			break;
		}
		else if (error == AVERROR_EOF)
		{
			break;
		}
		else if (error < 0 && error != AVERROR(EAGAIN))
		{
			throw runtime_error("avcodec_receive_frame() failed: " + AvStrError(error));
		}
    }

    return framePtr;
}

int32_t Stream::GetInterframeDelayInMilliseconds(double frameTimestamp)
{
	//double actual_delay;
	double delay = frameTimestamp - lastFrameTimestamp_; /* the pts from last time */
	if (delay <= 0 || delay >= 1.0)
	{
		/* if incorrect delay, use previous one */
		delay = lastFrameDelay_;
	}

	/* save for next time */
	lastFrameDelay_ = delay;
	lastFrameTimestamp_ = frameTimestamp;

	videoTimer_ += delay;
	/* computer the REAL delay */
	double actual_delay = videoTimer_ - (av_gettime() / 1000000.0);
	if (actual_delay < 0.01)
	{
		/* Really it should skip the picture instead */
		actual_delay = 0.01;
	}

	return static_cast<int32_t>(actual_delay * 1000 + 0.5);
}

string Stream::AvStrError(int errnum)
{
    char buf[128];
    av_strerror(errnum, buf, sizeof(buf));
    return string(buf);
}

void Stream::Stop()
{
    stopRequested_ = true;
    packetQueue_.StopWait();

    AVPacket *packetPtr = nullptr;
    while (packetQueue_.TryPop(packetPtr))
    {
        av_packet_free(&packetPtr);
    }

    if (workerThread_.joinable())
    {
        workerThread_.join();
    }
}