#include "streamplayer.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdexcept>

#define STREAMPLAYER_API extern "C" __declspec(dllexport)

FFmpeg::Facade::StreamPlayer player;

STREAMPLAYER_API int32_t __stdcall Initialize(FFmpeg::Facade::StreamPlayerParams params)
{
    try
    {
        player.Initialize(params);
    }
    catch (std::runtime_error &)
    {
        return 1;
    }

    return 0;
}

STREAMPLAYER_API int32_t __stdcall StartPlay(const char* url)
{
    try
    {
        player.StartPlay(url);
    }
    catch (std::runtime_error &)
    {
        return 1;
    }

    return 0;
}

/*STREAMPLAYER_API int32_t __stdcall Play()
{
    try
    {
        player.Play();
    }
    catch (std::runtime_error &)
    {
        return 1;
    }

    return 0;
}*/

STREAMPLAYER_API int32_t __stdcall GetCurrentFrame(uint8_t** bmp_ptr)
{
    try
    {
        player.GetCurrentFrame(bmp_ptr);
    }
    catch (std::runtime_error &)
    {
        return 1;
    }

    return 0;
}

STREAMPLAYER_API int32_t __stdcall GetFrameSize(uint32_t* widthPtr, uint32_t* heightPtr)
{
    try
    {
        player.GetFrameSize(widthPtr, heightPtr);
    }
    catch (std::runtime_error &)
    {
        return 1;
    }

    return 0;
}

STREAMPLAYER_API int32_t __stdcall Stop()
{
    try
    {
        player.Stop();
    }
    catch (std::runtime_error &)
    {
        return 1;
    }

    return 0;
}

STREAMPLAYER_API int32_t __stdcall Uninitialize()
{
    try
    {
        player.Uninitialize();
    }
    catch (std::runtime_error &)
    {
        return 1;
    }

    return 0;
}