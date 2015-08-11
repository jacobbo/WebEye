#include "streamplayer.h"
#include <cassert>

#include "decoder.h"

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

    ::SetWindowLongPtr(playerParams_.window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    originalWndProc_ = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(playerParams_.window, GWLP_WNDPROC,
        reinterpret_cast<LONG_PTR>(WndProc)));
}

void StreamPlayer::StartPlay(string const& streamUrl)
{
	workerThread_ = boost::thread(&StreamPlayer::Play, this, streamUrl);
}

void StreamPlayer::Play(string const& streamUrl)
{
	boost::unique_lock<boost::mutex> lock(mutex_, boost::defer_lock);
	if (!lock.try_lock())
	{
		// Skip subsequent calls until a stream fails or stopped.  
		return;
	}

	try
	{
		Decoder decoder(streamUrl);

		stopRequested_ = false;
		bool firstFrame = true;

		framePtr_.reset();
		
		for (;;)
		{
			decoder.GetNextFrame(framePtr_);

			if (stopRequested_ || framePtr_ == nullptr)
			{
				::PostMessage(playerParams_.window, WM_STREAMSTOPPED, 0, 0);
				break;
			}

			::PostMessage(playerParams_.window, WM_INVALIDATE, 0, 0);

			const auto millisecondsToWait = decoder.InterframeDelayInMilliseconds();
			boost::this_thread::sleep_for(boost::chrono::milliseconds(millisecondsToWait));

			if (firstFrame)
			{
				::PostMessage(playerParams_.window, WM_STREAMSTARTED, 0, 0);
				firstFrame = false;
			}
		}
	}
	catch (runtime_error&)
	{
		::PostMessage(playerParams_.window, WM_STREAMFAILED, 0, 0);
	}
}

void StreamPlayer::Stop()
{
    stopRequested_ = true;

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

void StreamPlayer::RaiseStreamStartedEvent()
{
	if (playerParams_.streamStartedCallback != nullptr)
		playerParams_.streamStartedCallback();
}

void StreamPlayer::RaiseStreamStoppedEvent()
{
	if (playerParams_.streamStoppedCallback != nullptr)
		playerParams_.streamStoppedCallback();
}

void StreamPlayer::RaiseStreamFailedEvent()
{
    if (playerParams_.streamFailedCallback != nullptr)
        playerParams_.streamFailedCallback();
}