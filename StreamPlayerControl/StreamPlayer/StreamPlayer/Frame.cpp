#include "frame.h"
#include <cassert>
#include <Objbase.h>

using namespace std;
using namespace boost;
using namespace FFmpeg;
using namespace FFmpeg::Facade;

Frame::Frame(uint32_t width, uint32_t height, AVPicture &avPicture)
	: width_(width), height_(height)
{
    int32_t lineSize = avPicture.linesize[0];
    uint32_t padding = GetPadding(lineSize);

    pixelsPtr_ = new uint8_t[height_ * (lineSize + padding)];

    Update(avPicture);
}

void Frame::Update(AVPicture &avPicture)
{
	unique_lock<mutex> lock(mutex_);

    int32_t lineSize = avPicture.linesize[0];
    uint32_t padding = GetPadding(lineSize);

	for (int32_t y = 0; y < height_; ++y)
	{
        ::CopyMemory(pixelsPtr_ + (lineSize + padding) * y,
            avPicture.data[0] + (height_ - y - 1) * lineSize, lineSize);

        ::SecureZeroMemory(pixelsPtr_ + (lineSize + padding) * y + lineSize, padding);
	}
}

void Frame::Draw(HWND window)
{
	unique_lock<mutex> lock(mutex_);

	PAINTSTRUCT ps;

	RECT rc = { 0, 0, 0, 0 };
	::GetClientRect(window, &rc);

	BITMAPINFO bmpInfo;
    ::SecureZeroMemory(&bmpInfo, sizeof(bmpInfo));
	bmpInfo.bmiHeader.biBitCount = 24;
	bmpInfo.bmiHeader.biHeight = height_;
	bmpInfo.bmiHeader.biWidth = width_;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biCompression = BI_RGB;

    HDC hdc = ::BeginPaint(window, &ps);
    assert(hdc != nullptr);

    ::SetStretchBltMode(hdc, HALFTONE);
	::StretchDIBits(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        0, 0, width_, height_, pixelsPtr_, &bmpInfo, DIB_RGB_COLORS, SRCCOPY);

	::EndPaint(window, &ps);
}

void Frame::ToBmp(uint8_t **bmpPtr)
{
	assert(bmpPtr != nullptr);

	unique_lock<mutex> lock(mutex_);

	*bmpPtr =
		static_cast<uint8_t *>(::CoTaskMemAlloc(sizeof(BITMAPINFOHEADER) + height_ * width_ * 3));

	if (*bmpPtr == nullptr)
		throw runtime_error("CoTaskMemAlloc failed");

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

Frame::~Frame()
{
	delete[] pixelsPtr_;
}