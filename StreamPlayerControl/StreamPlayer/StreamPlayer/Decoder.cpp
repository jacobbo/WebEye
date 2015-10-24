#include "decoder.h"
#include <stdexcept>

#include "streamplayer.h"
#include "frame.h"

using namespace std;
using namespace boost;
using namespace FFmpeg;
using namespace FFmpeg::Facade;

Decoder::Decoder(string const& streamUrl)
	: formatCtxPtr_(nullptr), codecCtxPtr_(nullptr),
	videoStreamIndex_(-1), imageConvertCtxPtr_(nullptr)
{
	static boost::once_flag flag = BOOST_ONCE_INIT;
	boost::call_once(flag, []()
	{
		av_register_all();
		avdevice_register_all();
		avcodec_register_all();
		avformat_network_init();
	});

	AVDictionary *streamOpts = nullptr;
	av_dict_set(&streamOpts, "stimeout", "5000000", 0); // 5 seconds timeout.

	formatCtxPtr_ = avformat_alloc_context();
	int error = avformat_open_input(&formatCtxPtr_, streamUrl.c_str(), nullptr, &streamOpts);
	if (error != 0)
	{
		av_dict_free(&streamOpts);
		throw runtime_error("avformat_open_input() failed: " + AvStrError(error));
	}

	av_dict_free(&streamOpts);

	error = avformat_find_stream_info(formatCtxPtr_, nullptr);
	if (error < 0)
	{
		avformat_close_input(&formatCtxPtr_);
		throw runtime_error("avformat_find_stream_info() failed: " + AvStrError(error));
	}

	for (uint32_t i = 0; i < formatCtxPtr_->nb_streams; i++)
	{
		if (formatCtxPtr_->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStreamIndex_ = i;
			break;
		}
	}

	if (videoStreamIndex_ == -1)
	{
		avformat_close_input(&formatCtxPtr_);
		throw runtime_error("no video stream");
	}

	codecCtxPtr_ = formatCtxPtr_->streams[videoStreamIndex_]->codec;
	AVCodec *codecPtr = avcodec_find_decoder(codecCtxPtr_->codec_id);
	if (codecPtr == nullptr)
	{
		avformat_close_input(&formatCtxPtr_);
		throw runtime_error("avcodec_find_decoder() failed");
	}

	error = avcodec_open2(codecCtxPtr_, codecPtr, nullptr);
	if (error < 0)
	{
		avcodec_close(codecCtxPtr_);
		avformat_close_input(&formatCtxPtr_);
		throw runtime_error("avcodec_open2() failed: " + AvStrError(error));
	}	
}

void Decoder::GetNextFrame(std::unique_ptr<Frame>& framePtr, const StreamPlayer &player)
{
	AVFrame *avframePtr = av_frame_alloc();
	AVPacket packet;

    while (!player.IsStopRequested())
	{
		int error = av_read_frame(formatCtxPtr_, &packet);
		if (error < 0)
		{
            framePtr.reset();

			if (error != static_cast<int>(AVERROR_EOF))
			{
				throw runtime_error("av_read_frame() failed: " + AvStrError(error));
			}

			// The end of a stream.
			break;
		}

		if (packet.stream_index == videoStreamIndex_)
		{
			int frameFinished = 0;			
			int bytesUsed = avcodec_decode_video2(codecCtxPtr_, avframePtr, &frameFinished, &packet);
            if (bytesUsed < 0)
            {
                av_free_packet(&packet);
                throw runtime_error("avcodec_decode_video2() failed: " + AvStrError(bytesUsed));
            }

			if (frameFinished != 0)
			{
				AVPicture avRgbFrame;
				AVPixelFormat pixelFormat = AV_PIX_FMT_BGR24;
				avpicture_alloc(&avRgbFrame, pixelFormat, codecCtxPtr_->width, codecCtxPtr_->height);

				if (imageConvertCtxPtr_ == nullptr)
				{					
					imageConvertCtxPtr_ = sws_getContext(codecCtxPtr_->width, codecCtxPtr_->height,
						codecCtxPtr_->pix_fmt, codecCtxPtr_->width, codecCtxPtr_->height,
						pixelFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);

					if (imageConvertCtxPtr_ == nullptr)
					{
						throw runtime_error("sws_getContext() failed");
					}
				}

				sws_scale(imageConvertCtxPtr_, ((AVPicture*)avframePtr)->data,
					((AVPicture*)avframePtr)->linesize, 0, codecCtxPtr_->height,
					avRgbFrame.data, avRgbFrame.linesize);

				if (framePtr == nullptr)
				{
					framePtr = make_unique<Frame>(codecCtxPtr_->width,
						codecCtxPtr_->height, avRgbFrame);
				}
				else
					framePtr->Update(avRgbFrame);

				avpicture_free(&avRgbFrame);
				av_frame_free(&avframePtr);
				av_free_packet(&packet);

				break;				
			}			
		}

		av_free_packet(&packet);
	}
}

int32_t Decoder::InterframeDelayInMilliseconds() const
{
	return codecCtxPtr_->ticks_per_frame * 1000 *
		codecCtxPtr_->time_base.num / codecCtxPtr_->time_base.den;
}

string Decoder::AvStrError(int errnum)
{
	char buf[128];
	av_strerror(errnum, buf, sizeof(buf));
	return string(buf);
}

Decoder::~Decoder()
{
	if (imageConvertCtxPtr_ != nullptr)
	{
		sws_freeContext(imageConvertCtxPtr_);
	}

	avcodec_close(codecCtxPtr_);
	avformat_close_input(&formatCtxPtr_);
	avformat_free_context(formatCtxPtr_);
}