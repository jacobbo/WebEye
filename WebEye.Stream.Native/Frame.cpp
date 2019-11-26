#include "frame.h"

#include <cassert>

#include <Objbase.h>

using namespace std;

using namespace WebEye;
using namespace WebEye::FFmpeg;
using namespace WebEye::FFmpeg::Facade;

Frame::Frame(uint32_t width, uint32_t height, double timestamp, AVFrame &avFrame)
    : width_(width), height_(height), timestamp_(timestamp)
{
    int32_t lineSize = avFrame.linesize[0];
    uint32_t padding = GetPadding(lineSize);

    pixelsPtr_ = new uint8_t[height_ * (lineSize + padding)];

    for (int32_t y = 0; y < height_; ++y)
    {
        ::CopyMemory(pixelsPtr_ + (lineSize + padding) * y,
            avFrame.data[0] + (height_ - y - 1) * lineSize, lineSize);

        ::SecureZeroMemory(pixelsPtr_ + (lineSize + padding) * y + lineSize, padding);
    }
}

void Frame::ToBmp(uint8_t **bmpPtr)
{
    assert(bmpPtr != nullptr);

    if (bmpPtr == nullptr)
        throw runtime_error("invalid argument");

    *bmpPtr = new uint8_t [sizeof(BITMAPINFOHEADER) + height_ * width_ * 3];

    //if (*bmpPtr == nullptr)
    //    throw runtime_error("CoTaskMemAlloc failed");

    BITMAPINFOHEADER *headerPtr = reinterpret_cast<BITMAPINFOHEADER *>(*bmpPtr);
    ::SecureZeroMemory(headerPtr, sizeof(BITMAPINFOHEADER));
    headerPtr->biBitCount = 24;
    headerPtr->biHeight = height_;
    headerPtr->biWidth = width_;
    headerPtr->biPlanes = 1;
    headerPtr->biSize = sizeof(BITMAPINFOHEADER);
    headerPtr->biCompression = BI_RGB;

    uint8_t* pixelsPtr = *bmpPtr + sizeof(BITMAPINFOHEADER);
    ::CopyMemory(pixelsPtr, pixelsPtr_, height_ * width_ * 3);
}