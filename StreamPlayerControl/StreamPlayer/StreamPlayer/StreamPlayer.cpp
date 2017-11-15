#include "streamplayer.h"
#include <cassert>

#define WM_INVALIDATE    WM_USER + 1
#define WM_STREAMSTARTED WM_USER + 2
#define WM_STREAMSTOPPED WM_USER + 3
#define WM_STREAMFAILED  WM_USER + 4

using namespace std;
using namespace boost;
using namespace FFmpeg;
using namespace FFmpeg::Facade;

WNDPROC StreamPlayer::originalWndProc_ = nullptr;

StreamPlayer::StreamPlayer()
	: stopRequested_(false) {}

void StreamPlayer::Initialize(StreamPlayerParams params)
{
	assert(params.window != nullptr);
    assert(params.streamStartedCallback != nullptr);
	assert(params.streamStoppedCallback != nullptr);
	assert(params.streamFailedCallback != nullptr);    
    
    playerParams_ = params;

    if (playerParams_.window == nullptr)
    {
        return;
    }

    ::SetWindowLongPtr(playerParams_.window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    originalWndProc_ = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(playerParams_.window, GWLP_WNDPROC,
        reinterpret_cast<LONG_PTR>(WndProc)));
}

void StreamPlayer::StartPlay(string const& streamUrl,
    uint32_t connectionTimeoutInMilliseconds, RtspTransport transport, RtspFlags flags)
{
	workerThread_ = boost::thread(&StreamPlayer::Play, this,
        streamUrl, connectionTimeoutInMilliseconds, transport, flags);
}

void StreamPlayer::Play(string const& streamUrl,
    int32_t connectionTimeoutInMilliseconds, RtspTransport transport, RtspFlags flags)
{
    boost::unique_lock<boost::mutex> lock(workerThreadMutex_, boost::defer_lock);
	if (!lock.try_lock())
	{
		// Skip subsequent calls until the stream fails or stopped.  
		return;
	}

	try
	{
        {
            unique_lock<mutex> lock1(streamMutex_);
            streamPtr_ = make_unique<Stream>(streamUrl,
				connectionTimeoutInMilliseconds, transport, flags);
        }

		streamPtr_->WaitForOpen();

		stopRequested_ = false;
		bool firstFrame = true;

		framePtr_.reset();
		
		for (;;)
		{
            unique_ptr<Frame> framePtr = streamPtr_->GetNextFrame();

			if (stopRequested_ || framePtr == nullptr)
			{
                if (playerParams_.window != nullptr)
                { 
				    ::PostMessage(playerParams_.window, WM_STREAMSTOPPED, 0, 0);
                }

				break;
			}
            else
            {
                unique_lock<mutex> lock1(frameMutex_);
                framePtr_.swap(framePtr);
            }

            if (playerParams_.window != nullptr)
            {
                ::PostMessage(playerParams_.window, WM_INVALIDATE, 0, 0);
            }

			if (firstFrame)
			{
                if (playerParams_.window != nullptr)
                {
                    ::PostMessage(playerParams_.window, WM_STREAMSTARTED, 0, 0);
                }
                
                firstFrame = false;
			}

            boost::this_thread::sleep_for(
                boost::chrono::milliseconds(streamPtr_->InterframeDelayInMilliseconds()));
		}

        {
            unique_lock<mutex> lock1(streamMutex_);
            streamPtr_.reset();
        }
	}
	catch (runtime_error& e)
	{
        {
            unique_lock<mutex> lock1(errorMutex_);
            error_ = e.what();
        }

        if (playerParams_.window != nullptr)
        {
            ::PostMessage(playerParams_.window, WM_STREAMFAILED, 0, 0);
        }
    }
}

void StreamPlayer::Stop()
{
    stopRequested_ = true;

    {
        unique_lock<mutex> lock(streamMutex_);
        if (streamPtr_ != nullptr)
        {
            streamPtr_->Stop();
        }
    }

    if (workerThread_.joinable())
        workerThread_.join();
}

void StreamPlayer::Uninitialize()
{
    Stop();

    if (playerParams_.window != nullptr && originalWndProc_ != nullptr)
    {
        // Clear the message queue.
        MSG msg;
        while (::PeekMessage(&msg, playerParams_.window, 0, 0, PM_REMOVE)) {}

        ::SetWindowLongPtr(playerParams_.window, GWLP_USERDATA, 0);
        ::SetWindowLongPtr(playerParams_.window, GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(originalWndProc_));
    }
}

void StreamPlayer::DrawFrame()
{
    unique_lock<mutex> lock(frameMutex_);

    if (framePtr_ != nullptr)
        framePtr_->Draw(playerParams_.window);
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

	case WM_STREAMSTARTED:
		playerPtr->RaiseStreamStartedEvent();
		break;

	case WM_STREAMSTOPPED:
		playerPtr->RaiseStreamStoppedEvent();
		break;    

	case WM_STREAMFAILED:
		playerPtr->RaiseStreamFailedEvent();
		break;

    default: break;
    }

    return CallWindowProc(originalWndProc_, hWnd, uMsg, wParam, lParam);
}

void StreamPlayer::GetCurrentFrame(uint8_t **bmpPtr)
{
    unique_lock<mutex> lock(frameMutex_);

    if (framePtr_ == nullptr)
        throw runtime_error("no frame");

    framePtr_->ToBmp(bmpPtr);
}

void StreamPlayer::GetFrameSize(uint32_t *widthPtr, uint32_t *heightPtr)
{
    assert(widthPtr != nullptr && heightPtr != nullptr);

    if (widthPtr == nullptr || heightPtr == nullptr)
    {
        throw runtime_error("invalid argument");
    }

    unique_lock<mutex> lock(frameMutex_);

    if (framePtr_ == nullptr)
    {
        throw runtime_error("no frame");
    }

    *widthPtr = framePtr_->Width();
    *heightPtr = framePtr_->Height();
}

void StreamPlayer::RaiseStreamStartedEvent()
{
    if (playerParams_.streamStartedCallback != nullptr)
    {
        playerParams_.streamStartedCallback();
    }
}

void StreamPlayer::RaiseStreamStoppedEvent()
{
    if (playerParams_.streamStoppedCallback != nullptr)
    {
        playerParams_.streamStoppedCallback();
    }
}

void StreamPlayer::RaiseStreamFailedEvent()
{
    if (playerParams_.streamFailedCallback != nullptr)
    {
        playerParams_.streamFailedCallback(error_.c_str());
    }
}