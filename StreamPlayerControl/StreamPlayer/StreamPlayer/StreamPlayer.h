#ifndef FFMPEG_FACADE_STREAMPLAYER_H
#define FFMPEG_FACADE_STREAMPLAYER_H

#include <string>
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "frame.h"

#define WM_INVALIDATE WM_USER + 1
#define WM_PLAYFAILED WM_INVALIDATE + 1
#define WM_PLAYSUCCEEDED WM_PLAYFAILED + 1

namespace FFmpeg
{

#pragma warning( push )
#pragma warning( disable : 4244 )

    extern "C"
    {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avio.h>
#include <libswscale/swscale.h>
    }

#pragma warning( pop )

    // The FFmpeg framework built using the following configuration:
    // ./configure --enable-asm --enable-yasm --arch=i386 --enable-static --disable-shared
    // --enable-version3 --enable-bzlib --enable-iconv --enable-zlib --toolchain=msvc

#pragma comment(lib, "libavformat.a")
#pragma comment(lib, "libavcodec.a")
#pragma comment(lib, "libavdevice.a")
#pragma comment(lib, "libavfilter.a")
#pragma comment(lib, "libavutil.a")
#pragma comment(lib, "libswresample.a")
#pragma comment(lib, "libswscale.a")
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "Vfw32.lib")

    namespace Facade
    {
        typedef void(__stdcall *PlaySucceededCallback)();
        typedef void(__stdcall *PlayFailedCallback)();

        struct StreamPlayerParams
        {
            StreamPlayerParams()
                : window(nullptr), playFailedCallback(nullptr), playSucceededCallback(nullptr) {}

            HWND window;
            PlayFailedCallback playFailedCallback;
            PlaySucceededCallback playSucceededCallback;            
        };


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
            /// <param name="params">A container window that video should be clipped to.</param>
            void Initialize(StreamPlayerParams params);

            /// <summary>
            /// Asynchronously plays a stream.
            /// </summary>
            /// <param name="url">The url of a stream to play.</param>
            void StartPlay(std::string const& url);

            /// <summary>
            /// Opens a stream.
            /// </summary>
            /// <param name="url">The url of a stream to open.</param>
            //void Open(std::string const& url);

            /// <summary>
            /// Plays the stream opened by the Open method.
            /// </summary>
            //void Play();

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

            void Render();
            void DrawFrame();
            void RaisePlaySucceededEvent();
            void RaisePlayFailedEvent();

            static LRESULT APIENTRY WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        private:

            //HWND window_;
            boost::atomic<bool> stopRequested_;
            std::string streamUrl_;
            StreamPlayerParams playerParams_;

            AVFormatContext *formatCtxPtr_;
            AVCodecContext  *codecCtxPtr_;
            int32_t videoStreamIndex_;

            std::unique_ptr<Frame> framePtr_;

            // There is a bug in the Visual Studio std::thread implementation,
            // which prohibits dll unloading, that is why the boost::thread is used instead.
            // https://connect.microsoft.com/VisualStudio/feedback/details/781665/stl-using-std-threading-objects-adds-extra-load-count-for-hosted-dll#tabs 

            boost::thread rendererThread_;
            static WNDPROC originalWndProc_;
        };
    }
}

#endif // FFMPEG_FACADE_STREAMPLAYER_H