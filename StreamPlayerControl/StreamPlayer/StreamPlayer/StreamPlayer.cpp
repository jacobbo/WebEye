#include "streamplayer.h"
#include <cassert>

using namespace std;
using namespace boost;
using namespace FFmpeg;
using namespace FFmpeg::Facade;

WNDPROC StreamPlayer::originalWndProc_ = nullptr;

StreamPlayer::StreamPlayer()
    : window_(nullptr), stopRequested_(false), formatCtxPtr_(nullptr),
    codecCtxPtr_(nullptr), videoStreamIndex_(-1) { }

void StreamPlayer::Initialize(HWND window)
{
    window_ = window;

    ::SetWindowLongPtr(window_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    originalWndProc_ = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(window_, GWLP_WNDPROC,
        reinterpret_cast<LONG_PTR>(WndProc)));
}

void StreamPlayer::Open(string const& url)
{
    Stop();

    stopRequested_ = false;

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

    AVFormatContext *formatCtxPtr = avformat_alloc_context();
    if (avformat_open_input(&formatCtxPtr, url.c_str(), nullptr, &streamOpts) != 0)
    {
        av_dict_free(&streamOpts);
        throw runtime_error("avformat_open_input failed");
    }

    av_dict_free(&streamOpts);

    if (avformat_find_stream_info(formatCtxPtr, nullptr) < 0)
    {
        avformat_close_input(&formatCtxPtr);
        throw runtime_error("avformat_find_stream_info failed");
    }

    int32_t streamIndex = -1;
    for (size_t i = 0; i < formatCtxPtr->nb_streams; i++)
    {
        if (formatCtxPtr->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            streamIndex = i;
            break;
        }
    }

    if (streamIndex == -1)
    {
        avformat_close_input(&formatCtxPtr);
        throw runtime_error("no video stream");
    }

    AVCodecContext *codecCtxPtr = formatCtxPtr->streams[streamIndex]->codec;
    AVCodec *pCodec = avcodec_find_decoder(codecCtxPtr->codec_id);
    if (pCodec == nullptr)
    {
        avformat_close_input(&formatCtxPtr);
        throw runtime_error("avcodec_find_decoder failed");
    }

    if (avcodec_open2(codecCtxPtr, pCodec, nullptr) < 0)
    {
        avcodec_close(codecCtxPtr);
        avformat_close_input(&formatCtxPtr);
        throw runtime_error("avcodec_open2 failed");
    }

    formatCtxPtr_ = formatCtxPtr;
    codecCtxPtr_ = codecCtxPtr;
    videoStreamIndex_ = streamIndex;
}

void StreamPlayer::Play()
{
    rendererThread_ = boost::thread(&StreamPlayer::Render, this);
}

void StreamPlayer::Render()
{
    framePtr_.reset();

    AVPixelFormat pixelFormat = AV_PIX_FMT_BGR24;

    AVPicture avRgbFrame;
    avpicture_alloc(&avRgbFrame, pixelFormat, codecCtxPtr_->width, codecCtxPtr_->height);

    SwsContext *imgConvertCtx = sws_getContext(codecCtxPtr_->width, codecCtxPtr_->height,
        codecCtxPtr_->pix_fmt, codecCtxPtr_->width, codecCtxPtr_->height,
        pixelFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);

    AVPacket packet;
    AVFrame *avframePtr = ::av_frame_alloc();

    for (;;)
    {
        if (stopRequested_)
        {
            break;
        }

        if (av_read_frame(formatCtxPtr_, &packet) < 0)
        {
            break;
        }

        if (packet.stream_index == videoStreamIndex_)
        {
            int32_t frameFinished = 0;
            avcodec_decode_video2(codecCtxPtr_, avframePtr, &frameFinished, &packet);

            if (frameFinished != 0)
            {
                sws_scale(imgConvertCtx, ((AVPicture*)avframePtr)->data,
                    ((AVPicture*)avframePtr)->linesize, 0, codecCtxPtr_->height,
                    avRgbFrame.data, avRgbFrame.linesize);

                if (framePtr_ == nullptr)
                {
                    Frame *framePtr = new Frame(codecCtxPtr_->width,
                        codecCtxPtr_->height, avRgbFrame);
                    framePtr_.reset(framePtr);
                }
                else
                    framePtr_->Update(avRgbFrame);

                ::PostMessage(window_, WM_INVALIDATE, 0, 0);

                av_free_packet(&packet);

                const auto millisecondsToWait = codecCtxPtr_->ticks_per_frame * 1000 *
                    codecCtxPtr_->time_base.num / codecCtxPtr_->time_base.den;
                boost::this_thread::sleep_for(boost::chrono::milliseconds(millisecondsToWait));
            }
        }
    }

    sws_freeContext(imgConvertCtx);
    av_frame_free(&avframePtr);
    avpicture_free(&avRgbFrame);
    av_free_packet(&packet);
    avcodec_close(codecCtxPtr_);
    avformat_close_input(&formatCtxPtr_);
    avformat_free_context(formatCtxPtr_);
}

void StreamPlayer::Stop()
{
    stopRequested_ = true;

    if (rendererThread_.joinable())
        rendererThread_.join();
}

void StreamPlayer::Uninitialize()
{
    Stop();

    if (window_ != INVALID_HANDLE_VALUE && originalWndProc_ != nullptr)
    {
        // clear message queue
        MSG msg;
        while (::PeekMessage(&msg, window_, 0, 0, PM_REMOVE)) {}

        ::SetWindowLongPtr(window_, GWLP_USERDATA, 0);
        ::SetWindowLongPtr(window_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(originalWndProc_));
    }
}

void StreamPlayer::DrawFrame()
{
    if (framePtr_ != nullptr)
        framePtr_->Draw(window_);
}

LRESULT APIENTRY StreamPlayer::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static StreamPlayer *playerPtr = nullptr;
    if (playerPtr == nullptr)
    {
        playerPtr = reinterpret_cast<StreamPlayer *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        assert(playerPtr != nullptr);
    }

    switch (uMsg)
    {
    case WM_INVALIDATE:
        ::InvalidateRect(hWnd, nullptr, FALSE);
        break;

    case WM_PAINT:
        playerPtr->DrawFrame();
        break;

    default: break;
    }

    return CallWindowProc(originalWndProc_, hWnd, uMsg, wParam, lParam);
}

void StreamPlayer::GetCurrentFrame(uint8_t **bmpPtr)
{
    if (framePtr_ == nullptr)
        throw runtime_error("no frame");

    framePtr_->ToBmp(bmpPtr);
}

void StreamPlayer::GetFrameSize(uint32_t *widthPtr, uint32_t *heightPtr)
{
    assert(widthPtr != nullptr && heightPtr != nullptr);

    if (framePtr_ == nullptr)
        throw runtime_error("no frame");

    *widthPtr = framePtr_->Width();
    *heightPtr = framePtr_->Height();
}