#ifndef WEBEYE_FFMPEG_FACADE_PLAYER_H
#define WEBEYE_FFMPEG_FACADE_PLAYER_H

#include <string>
#include <memory>

#pragma warning(push)
#pragma warning(disable: 4793)
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#pragma warning(pop)

#include "stream.h"
#include "frame.h"

namespace WebEye
{
    namespace FFmpeg
    {
        namespace Facade
        {
            class Stream;

            typedef void(__stdcall *StreamStoppedCallback)();
            typedef void(__stdcall *StreamFailedCallback)(const char* error);
            typedef void(__stdcall *FrameRecievedCallback)(uint8_t *bmpPtr);

            struct PlayerParams
            {
                PlayerParams() : streamStoppedCallback(nullptr), streamFailedCallback(nullptr),
                    frameRecievedCallback(nullptr) {}

                StreamStoppedCallback streamStoppedCallback;
                StreamFailedCallback streamFailedCallback;
                FrameRecievedCallback frameRecievedCallback;
            };

            enum RtspTransport : int32_t { Undefined = 0, Udp = 1, Tcp = 2, UdpMulticast = 3, Http = 4 };

            enum RtspFlags : int32_t { None = 0, FilterSrc = 1, Listen = 2, PreferTcp = 3 };

            /// <summary>
            /// A Player class implements a stream playback functionality.
            /// </summary>
            class Player
            {
            public:
                Player(const Player&) = delete;
                Player& operator=(const Player&) = delete;

                Player() {}

                /// <summary>
                /// Initializes the player.
                /// </summary>
                /// <param name="playerParams">The PlayerParams object that contains the information that is used to initialize the player.</param>
                void Initialize(PlayerParams playerParams);

                /// <summary>
                /// Asynchronously plays a stream.
                /// </summary>
                /// <param name="streamUrl">The url of a stream to play.</param>
                /// <param name="connectionTimeoutInMilliseconds">The connection timeout in milliseconds.</param>
                /// <param name="streamTimeoutInMilliseconds">The stream timeout in milliseconds.</param>
                /// <param name="transport">RTSP transport protocol.</param>
                /// <param name="flags">RTSP flags.</param>
                void StartPlay(std::string const& streamUrl, int32_t connectionTimeoutInMilliseconds,
                    int32_t streamTimeoutInMilliseconds, RtspTransport transport, RtspFlags flags);

                /// <summary>
                /// Pauses a network-based stream.
                /// </summary>
                void Pause();

                /// <summary>
                /// Resumes a network-based stream.
                /// </summary>
                void Resume();

                /// <summary>
                /// Stops a stream.
                /// </summary>
                void Stop();

                /// <summary>
                /// Uninitializes the player.
                /// </summary>
                void Uninitialize();

            private:
                /// <summary>
                /// Plays a stream.
                /// </summary>
                /// <param name="streamUrl">The url of a stream to play.</param>
                /// <param name="connectionTimeoutInMilliseconds">The connection timeout in milliseconds.</param>
                /// <param name="streamTimeoutInMilliseconds">The stream timeout in milliseconds.</param>
                /// <param name="transport">RTSP transport protocol.</param>
                /// <param name="transport">RTSP flags.</param>			
                void Play(std::string const& streamUrl, int32_t connectionTimeoutInMilliseconds,
                    int32_t streamTimeoutInMilliseconds, RtspTransport transport, RtspFlags flags);

                /// <summary>
                /// Raises the StreamStopped event.
                /// </summary>
                void RaiseStreamStoppedEvent();

                /// <summary>
                /// Raises the StreamFailed event.
                /// </summary>
                void RaiseStreamFailedEvent();

                /// <summary>
                /// Raises the FrameReceived event.
                /// </summary>
                void RaiseFrameReceivedEvent();

            private:

                PlayerParams playerParams_;

                boost::mutex streamMutex_;
                std::unique_ptr<Stream> stream_;

                boost::mutex frameMutex_;
                std::unique_ptr<Frame> frame_;

                boost::mutex workerThreadMutex_;
                boost::thread workerThread_;

                boost::mutex errorMutex_;
                std::string error_;
            };
        }
    }
}

#endif // WEBEYE_FFMPEG_FACADE_PLAYER_H