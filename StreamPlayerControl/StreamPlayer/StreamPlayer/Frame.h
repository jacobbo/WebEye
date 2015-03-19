#ifndef FFMPEG_FACADE_FRAME_H
#define FFMPEG_FACADE_FRAME_H

#include <cstdint>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace FFmpeg
{

#pragma warning( push )
#pragma warning( disable : 4244 )

    extern "C"
    {
#include <libavcodec/avcodec.h>
    }

#pragma warning( pop )

    namespace Facade
    {
        /// <summary>
        /// A Frame class implements a set of frame-related utilities. 
        /// </summary>
        class Frame : private boost::noncopyable
        {
        public:
            /// <summary>
            /// Initializes a new instance of the Frame class.
            /// </summary>
            Frame(uint32_t width, uint32_t height, AVPicture &avPicture);

            /// <summary>
            /// Gets the width, in pixels, of the frame.
            /// </summary>
            uint32_t Width() const { return width_; }

            /// <summary>
            /// Gets the height, in pixels, of the frame.
            /// </summary>
            uint32_t Height() const { return height_; }

            /// <summary>
            /// Updates the frame.
            /// </summary>
            /// <param name="avPicture">The AVPicture object to update the frame with.</param>
            void Update(AVPicture &avPicture);

            /// <summary>
            /// Draws the frame.
            /// </summary>
            /// <param name="window">A container window that frame should be drawn on.</param>
            void Draw(HWND window);

            /// <summary>
            /// Converts the frame to a bitmap.
            /// </summary>
            /// <param name="bmpPtr">Address of a pointer to a byte that will receive the DIB.</param>
            void ToBmp(uint8_t **bmpPtr);

            /// <summary>
            /// Releases all resources used by the frame.
            /// </summary>
            ~Frame()
            {
                delete[] pixelsPtr_;
            }

        private:
            /// <summary>
            /// The bits in the array are packed together, but each scan line must be
            /// padded with zeros to end on a LONG data-type boundary.
            /// </summary>
            uint32_t GetPadding(int32_t lineSize) const
            {
                return lineSize % sizeof(uint32_t) > 0 ?
                    sizeof(uint32_t) - (lineSize % sizeof(uint32_t)) : 0;
            }

            int32_t width_, height_;
            uint8_t *pixelsPtr_;
            boost::mutex mutex_;
        };
    }
}

#endif // FFMPEG_FACADE_FRAME_H