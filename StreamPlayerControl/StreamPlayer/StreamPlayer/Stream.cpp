#include "stream.h"
#include <stdexcept>

#include <boost/lexical_cast.hpp>

#include "frame.h"
#include "StreamPlayer.h"

using namespace std;
using namespace boost;
using namespace FFmpeg;
using namespace FFmpeg::Facade;

Stream::Stream(string const& streamUrl,
	int32_t connectionTimeoutInMilliseconds, RtspTransport transport, RtspFlags flags)
    : url_(streamUrl), connectionTimeout_(connectionTimeoutInMilliseconds),
	transport_(transport), flags_(flags), openedOrFailed_(false),
	stopRequested_(false), videoStreamIndex_(-1)
{
}

void FFmpeg::Facade::Stream::WaitForOpen()
{
	workerThread_ = thread(&Stream::OpenAndRead, this);

	boost::unique_lock<mutex> lock(mutex_);
	conditionVariable_.wait(lock, [this] { return openedOrFailed_; });

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

    {
        boost::lock_guard<boost::mutex> lock(streamPtr->mutex_);
        if (streamPtr->openedOrFailed_)
        {
            return 0;
        }
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - streamPtr->connectionStart_);
    
    if (elapsed > streamPtr->connectionTimeout_)
    {
        return 1;
    }

    return 0;
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

	unique_ptr<AVFormatContext, std::function<void(AVFormatContext*)>>
		formatCtxPtr(avformat_alloc_context(), [](AVFormatContext* ptr)
	{
		avformat_close_input(&ptr);
		avformat_free_context(ptr);
	});

	formatCtxPtr->interrupt_callback.callback = InterruptCallback;
	formatCtxPtr->interrupt_callback.opaque = this;
	formatCtxPtr->flags |= AVFMT_FLAG_NONBLOCK;

    connectionStart_ = std::chrono::system_clock::now();
    
	auto rawPtr = formatCtxPtr.get();
    int error = avformat_open_input(&rawPtr, url_.c_str(), nullptr, nullptr);
    if (error != 0)
    {
        throw runtime_error("avformat_open_input() failed: " + AvStrError(error));
    }

    error = avformat_find_stream_info(formatCtxPtr.get(), nullptr);
    if (error < 0)
    {
        throw runtime_error("avformat_find_stream_info() failed: " + AvStrError(error));
    }

	AVStream *videoStreamPtr = nullptr;
    for (uint32_t i = 0; i < formatCtxPtr->nb_streams; i++)
    {
        if (formatCtxPtr->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex_ = i;
			videoStreamPtr = formatCtxPtr->streams[i];
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
		codecCtxPtr(avcodec_alloc_context3(codecPtr), [](AVCodecContext* ptr)
	{
		avcodec_free_context(&ptr);
	});

	error = avcodec_parameters_to_context(codecCtxPtr.get(), videoStreamPtr->codecpar);
	if (error < 0)
	{
		throw runtime_error("avcodec_parameters_to_context() failed: " + AvStrError(error));
	}

	AVDictionary *optionsPtr = nullptr;
	if (transport_ == RtspTransport::Http)
	{
		av_dict_set(&optionsPtr, "rtsp_transport", "http", 0);
	}
	else if (transport_ == RtspTransport::Tcp)
	{
		av_dict_set(&optionsPtr, "rtsp_transport", "tcp", 0);
	}
	else if (transport_ == RtspTransport::Udp)
	{
		av_dict_set(&optionsPtr, "rtsp_transport", "udp", 0);
	}
	else if (transport_ == RtspTransport::UdpMulticast)
	{
		av_dict_set(&optionsPtr, "rtsp_transport", "udp_multicast", 0);
	}

	if (flags_ == RtspFlags::FilterSrc)
	{
		av_dict_set(&optionsPtr, "rtsp_flags", "filter_src", 0);
	}
	else if (flags_ == RtspFlags::Listen)
	{
		av_dict_set(&optionsPtr, "rtsp_flags", "listen", 0);
	}
	else if (flags_ == RtspFlags::PreferTcp)
	{
		av_dict_set(&optionsPtr, "rtsp_flags", "prefer_tcp", 0);
	}

    error = avcodec_open2(codecCtxPtr.get(), codecPtr, &optionsPtr);
	av_dict_free(&optionsPtr);
    if (error < 0)
    {
        throw runtime_error("avcodec_open2() failed: " + AvStrError(error));
    }

	formatCtxPtr_.swap(formatCtxPtr);
	codecCtxPtr_.swap(codecCtxPtr);
}

void Stream::Read()
{
    while (!stopRequested_)
    {
		unique_ptr<AVPacket, std::function<void(AVPacket*)>>
			packetPtr(av_packet_alloc(), [](AVPacket* ptr)
		{
			av_packet_free(&ptr);
		});

        int error = av_read_frame(formatCtxPtr_.get(), packetPtr.get());
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

    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        openedOrFailed_ = true;
    }

    conditionVariable_.notify_one();

    if (IsOpen())
    {
		Read();
    }    
}

bool FFmpeg::Facade::Stream::IsOpen() const
{
	return formatCtxPtr_ != nullptr && codecCtxPtr_ != nullptr && videoStreamIndex_ > -1;
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
		codecCtxPtr_->width, codecCtxPtr_->height, pixelFormat, 1);

	if (imageConvertCtxPtr_ == nullptr)
	{
		auto rawPtr = sws_getContext(codecCtxPtr_->width, codecCtxPtr_->height,
			codecCtxPtr_->pix_fmt, codecCtxPtr_->width, codecCtxPtr_->height,
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

		imageConvertCtxPtr_.swap(imageConvertCtxPtr);
	}

	sws_scale(imageConvertCtxPtr_.get(), avframePtr->data,
		avframePtr->linesize, 0, codecCtxPtr_->height,
		avRgbFramePtr->data, avRgbFramePtr->linesize);

	unique_ptr<Frame> framePtr = make_unique<Frame>(codecCtxPtr_->width,
		codecCtxPtr_->height, *avRgbFramePtr);

	return framePtr;
}

unique_ptr<Frame> Stream::GetNextFrame()
{
    unique_ptr<Frame> framePtr;    

    while (!stopRequested_)
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

		int error = avcodec_send_packet(codecCtxPtr_.get(), packetPtr.get());
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
		error = avcodec_receive_frame(codecCtxPtr_.get(), avframePtr.get());
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

int32_t Stream::InterframeDelayInMilliseconds() const
{
    return codecCtxPtr_->ticks_per_frame * 1000 *
        codecCtxPtr_->time_base.num / codecCtxPtr_->time_base.den;
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