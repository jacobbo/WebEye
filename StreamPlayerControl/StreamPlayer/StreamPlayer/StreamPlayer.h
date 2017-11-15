#ifndef FFMPEG_FACADE_STREAMPLAYER_H
#define FFMPEG_FACADE_STREAMPLAYER_H

#include <string>
#include <memory>
#include <boost/noncopyable.hpp>

#pragma warning( push )
#pragma warning( disable : 4100 )

#include <boost/thread.hpp>

#pragma warning( pop )

#include <boost/atomic.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "stream.h"
#include "frame.h"

namespace FFmpeg
{
    namespace Facade
    {
        class Stream;

        typedef void(__stdcall *StreamStartedCallback)();
		typedef void(__stdcall *StreamStoppedCallback)();
        typedef void(__stdcall *StreamFailedCallback)(const char* error);

        struct StreamPlayerParams
        {
            StreamPlayerParams()
				: window(nullptr), streamStartedCallback(nullptr),
				streamStoppedCallback(nullptr), streamFailedCallback(nullptr) {}

            HWND window;
            StreamStartedCallback streamStartedCallback;
			StreamStoppedCallback streamStoppedCallback;
			StreamFailedCallback streamFailedCallback;
        };

		enum RtspTransport : int32_t { Undefined = 0, Udp = 1, Tcp = 2, UdpMulticast = 3, Http = 4 };

		enum RtspFlags : int32_t { None = 0, FilterSrc = 1, Listen = 2, PreferTcp = 3 };

        /// <summary>
        /// A StreamPlayer class implements a stream playback functionality.
        /// </summary>
        class StreamPlayer : private boost::noncopyable
        {
        public:

            /// <summary>
            /// Initializes a new instance of the StreamPlayer class.
            /// </summary>
            StreamPlayer();

            /// <summary>
            /// Initializes the player.
            /// </summary>
			/// <param name="playerParams">The StreamPlayerParams object that contains the information that is used to initialize the player.</param>
			void Initialize(StreamPlayerParams playerParams);

            /// <summary>
            /// Asynchronously plays a stream.
            /// </summary>
            /// <param name="streamUrl">The url of a stream to play.</param>
            /// <param name="connectionTimeoutInMilliseconds">The connection timeout in milliseconds.</param>
			/// <param name="transport">RTSP transport protocol.</param>
			/// <param name="transport">RTSP flags.</param>
            void StartPlay(std::string const& streamUrl,
                uint32_t connectionTimeoutInMilliseconds, RtspTransport transport, RtspFlags flags);

            /// <summary>
            /// Stops a stream.
            /// </summary>
            void Stop();

            /// <summary>
            /// Retrieves the current frame being displayed by the player.
            /// </summary>
            /// <param name="bmpPtr">Address of a pointer to a byte that will receive the DIB.</param>
            void GetCurrentFrame(uint8_t **bmpPtr);

            /// <summary>
            /// Retrieves the unstretched frame size, in pixels.
            /// </summary>
            /// <param name="widthPtr">A pointer to an int that will receive the width.</param>
            /// <param name="heightPtr">A pointer to an int that will receive the height.</param>
            void GetFrameSize(uint32_t *widthPtr, uint32_t *heightPtr);

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
			/// <param name="transport">RTSP transport protocol.</param>
			/// <param name="transport">RTSP flags.</param>
            void Play(std::string const& streamUrl,
                int32_t connectionTimeoutInMilliseconds, RtspTransport transport, RtspFlags flags);

			/// <summary>
			/// Draws a frame.
			/// </summary>
            void DrawFrame();

			/// <summary>
			/// Raises the StreamStarted event.
			/// </summary>
            void RaiseStreamStartedEvent();

			/// <summary>
			/// Raises the StreamStopped event.
			/// </summary>
			void RaiseStreamStoppedEvent();

			/// <summary>
			/// Raises the StreamFailed event.
			/// </summary>
            void RaiseStreamFailedEvent();

            static LRESULT APIENTRY WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        private:
			            
            boost::atomic<bool> stopRequested_;
            StreamPlayerParams playerParams_;

            boost::mutex streamMutex_;
            std::unique_ptr<Stream> streamPtr_;

            boost::mutex frameMutex_;
            std::unique_ptr<Frame> framePtr_;

            // There is a bug in the Visual Studio std::thread implementation,
            // which prohibits dll unloading, that is why the boost::thread is used instead.
            // https://connect.microsoft.com/VisualStudio/feedback/details/781665/stl-using-std-threading-objects-adds-extra-load-count-for-hosted-dll#tabs 

            boost::mutex workerThreadMutex_;
			boost::thread workerThread_;

            boost::mutex errorMutex_;
            std::string error_;
          
            static WNDPROC originalWndProc_;
        };
    }
}

#endif // FFMPEG_FACADE_STREAMPLAYER_H