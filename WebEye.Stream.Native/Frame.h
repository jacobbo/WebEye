#ifndef WEBEYE_FFMPEG_FACADE_FRAME_H
#define WEBEYE_FFMPEG_FACADE_FRAME_H

#include <cstdint>
#include <stdexcept>

namespace WebEye
{
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
			class Frame
			{
			public:
				Frame(const Frame&) = delete;
				Frame& operator=(const Frame&) = delete;

				/// <summary>
				/// Initializes a new instance of the Frame class.
				/// </summary>
				/// <param name="width">The width of the frame, in pixels.</param>
				/// <param name="height">The height of the frame, in pixels.</param> 
				/// <param name="timestamp">The presentation time stamp of the frame.</param> 
				/// <param name="avFrame">The AVFrame object to initialize the frame with.</param>
				Frame(uint32_t width, uint32_t height, double timestamp, AVFrame &avFrame);

				/// <summary>
				/// Gets the presentation time stamp of the frame.
				/// </summary>
				double Timestamp() const { return timestamp_; }

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
					return lineSize % 4 > 0 ? 4 - (lineSize % 4) : 0;
				}

				int32_t width_, height_;
				double timestamp_;
				uint8_t *pixelsPtr_;
			};
		}
	}
}

#endif // WEBEYE_FFMPEG_FACADE_FRAME_H