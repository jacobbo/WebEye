#include "player.h"
#include <cassert>
#include <boost/chrono.hpp>

using namespace std;
//using namespace boost;

using namespace WebEye;
using namespace WebEye::FFmpeg;
using namespace WebEye::FFmpeg::Facade;

namespace boost {
	struct thread::dummy {};
}

void Player::Initialize(PlayerParams params)
{
	assert(params.streamStoppedCallback != nullptr);
	assert(params.streamFailedCallback != nullptr);
	assert(params.frameRecievedCallback != nullptr);
    
    playerParams_ = params;
}

void Player::StartPlay(string const& streamUrl, int32_t connectionTimeoutInMilliseconds,
	int32_t streamTimeoutInMilliseconds, RtspTransport transport, RtspFlags flags)
{
	workerThread_ = boost::thread(&Player::Play, this, streamUrl,
		connectionTimeoutInMilliseconds, streamTimeoutInMilliseconds, transport, flags);
}

void Player::Play(string const& streamUrl, int32_t connectionTimeoutInMilliseconds,
    int32_t streamTimeoutInMilliseconds, RtspTransport transport, RtspFlags flags)
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
			boost::unique_lock<boost::mutex> lock1(streamMutex_);
            stream_ = make_unique<Stream>(streamUrl,
				connectionTimeoutInMilliseconds, streamTimeoutInMilliseconds, transport, flags);
        }

		stream_->WaitForOpen();

		frame_.reset();
		
		for (;;)
		{
            std::unique_ptr<Frame> frame = stream_->GetNextFrame();
			double timestamp = 0.0;

			if (frame == nullptr)
			{
				RaiseStreamStoppedEvent();
				break;
			}
            else
            {
				timestamp = frame->Timestamp();

				boost::unique_lock<boost::mutex> lock1(frameMutex_);
                frame_.swap(frame);

				RaiseFrameReceivedEvent();
            }

			boost::this_thread::sleep_for(
                boost::chrono::milliseconds(stream_->GetInterframeDelayInMilliseconds(timestamp)));
		}

        {
			boost::unique_lock<boost::mutex> lock1(streamMutex_);
            stream_.reset();
        }
	}
	catch (runtime_error& e)
	{
        {
			boost::unique_lock<boost::mutex> lock1(errorMutex_);
            error_ = e.what();
        }

		RaiseStreamFailedEvent();
    }
}

void Player::Stop()
{
    {
		boost::unique_lock<boost::mutex> lock(streamMutex_);
        if (stream_ != nullptr)
        {
            stream_->Stop();
        }
    }

    if (workerThread_.joinable())
        workerThread_.join();
}

void Player::Uninitialize()
{
    Stop();
}

void Player::RaiseStreamStoppedEvent()
{
    if (playerParams_.streamStoppedCallback != nullptr)
    {
        playerParams_.streamStoppedCallback();
    }
}

void Player::RaiseStreamFailedEvent()
{
    if (playerParams_.streamFailedCallback != nullptr)
    {
        playerParams_.streamFailedCallback(error_.c_str());
    }
}

void Player::RaiseFrameReceivedEvent()
{
	if (playerParams_.frameRecievedCallback != nullptr)
	{
		uint8_t *bmpPtr = nullptr;
		frame_->ToBmp(&bmpPtr);

		playerParams_.frameRecievedCallback(bmpPtr);
	}
}