#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <msclr\marshal_cppstd.h>
#include <msclr\marshal.h>

#include "ManagedWrapper.h"

#include "DirectShow.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;
using namespace System::Drawing;

using namespace std;
using namespace WebEye;
using namespace WebEye::FFmpeg::Facade;

ManagedWrapper::ManagedWrapper()
{
	FramePtrRecievedHandler^ frameRecievedHandler =
		gcnew FramePtrRecievedHandler(this, &ManagedWrapper::RaiseFrameRecievedEvent);
	frameRecievedHandle_ = GCHandle::Alloc(frameRecievedHandler);
	IntPtr intPtr = Marshal::GetFunctionPointerForDelegate(frameRecievedHandler);
	FrameRecievedCallback frameRecievedCallback = static_cast<FrameRecievedCallback>(intPtr.ToPointer());

	StreamErrorHandler^ streamErrorHandler =
		gcnew StreamErrorHandler(this, &ManagedWrapper::RaiseStreamFailedEvent);
	streamErrorHandle_ = GCHandle::Alloc(streamErrorHandler);
	intPtr = Marshal::GetFunctionPointerForDelegate(streamErrorHandler);
	StreamFailedCallback streamFailedCallback = static_cast<StreamFailedCallback>(intPtr.ToPointer());

	StreamStoppedHandler^ streamStoppedHandler =
		gcnew StreamStoppedHandler(this, &ManagedWrapper::RaiseStreamStoppedEvent);
	streamStoppedHandle_ = GCHandle::Alloc(streamStoppedHandler);
	intPtr = Marshal::GetFunctionPointerForDelegate(streamStoppedHandler);
	StreamStoppedCallback streamStoppedCallback = static_cast<StreamStoppedCallback>(intPtr.ToPointer());

	playerPtr_ = new FFmpeg::Facade::Player();
	FFmpeg::Facade::PlayerParams params;
	params.frameRecievedCallback = frameRecievedCallback;
	params.streamFailedCallback = streamFailedCallback;
	params.streamStoppedCallback = streamStoppedCallback;

	playerPtr_->Initialize(params);
}

ManagedWrapper::ManagedWrapper(String^ deviceName) : ManagedWrapper()
{
	deviceName_ = deviceName;
}

ManagedWrapper::ManagedWrapper(Uri^ uri, TimeSpan^ connectionTimeout,
	TimeSpan^ streamTimeout, WebEye::RtspTransport transport,
	WebEye::RtspFlags flags) : ManagedWrapper()
{
	url_ = uri->IsFile ? uri->LocalPath : uri->ToString();
	connectionTimeout_ = static_cast<int>(connectionTimeout->TotalMilliseconds);
	streamTimeout_ = static_cast<int>(streamTimeout->TotalMilliseconds);
	transport_ = transport;
	flags_ = flags;
}

ManagedWrapper^ ManagedWrapper::FromUri(System::Uri ^ uri,
	System::TimeSpan ^ connectionTimeout, System::TimeSpan ^ streamTimeout,
	WebEye::RtspTransport transport, WebEye::RtspFlags flags)
{
	return gcnew ManagedWrapper(uri, connectionTimeout, streamTimeout, transport, flags);
}

String^ ManagedWrapper::Name::get()
{
	if (!String::IsNullOrEmpty(url_))
	{
		return url_;
	}
	else if (!String::IsNullOrEmpty(deviceName_))
	{
		return deviceName_;
	}

	throw gcnew System::InvalidOperationException();
}

void ManagedWrapper::Start()
{
	FFmpeg::Facade::RtspTransport transport =
		static_cast<FFmpeg::Facade::RtspTransport>(transport_);

	FFmpeg::Facade::RtspFlags flags =
		static_cast<FFmpeg::Facade::RtspFlags>(flags_);

	if (!String::IsNullOrEmpty(url_))
	{
		System::String^ managed = url_;
		const std::string url = msclr::interop::marshal_as<std::string>(managed);

		playerPtr_->StartPlay(url, connectionTimeout_, streamTimeout_,
			transport, flags);
	}
	else if (!String::IsNullOrEmpty(deviceName_))
	{
		String^ streamUrl = gcnew String("video=") + deviceName_;
		const std::string url = msclr::interop::marshal_as<std::string>(streamUrl);
		playerPtr_->StartPlay(url, connectionTimeout_, streamTimeout_,
			transport, flags);
	}
}

void ManagedWrapper::Pause()
{
    playerPtr_->Pause();
}

void ManagedWrapper::Resume()
{
    playerPtr_->Resume();
}

void ManagedWrapper::Stop()
{
	playerPtr_->Stop();
}

//void ManagedWrapper::SaveDevice(const Device::DeviceInfo *infoPtr,
//	std::vector<std::wstring> *namesPtr)
//{
//	std::wstring name(infoPtr->FriendlyName, SysStringLen(infoPtr->FriendlyName));
//	::SysFreeString(infoPtr->FriendlyName);
//	namesPtr->push_back(name);
//	/*IntPtr ip(infoPtr->FriendlyName);
//
//	String^ deviceName = Marshal::PtrToStringBSTR(ip);
//	Marshal::FreeBSTR(ip);
//	ManagedWrapper^ stream  = gcnew ManagedWrapper(deviceName);*/
//	//localStreams_->Add(stream);
//}

IEnumerable<ManagedWrapper^>^ ManagedWrapper::GetLocalStreams()
{
	//localStreams_->Clear();
	DirectShow dshow;
	vector<wstring> devices = dshow.ListVideoInputDevices();
	//EnumDevices(devices);

	//List<ManagedWrapper^>^ localStreams =
	//	gcnew System::Collections::Generic::List<ManagedWrapper^>();

	//EnumDevicesHandler^ enumDevicesHandler =
	//	gcnew EnumDevicesHandler(&ManagedWrapper::SaveDevice);
	//GCHandle enumDevicesHandle = GCHandle::Alloc(enumDevicesHandler);
	//IntPtr intPtr = Marshal::GetFunctionPointerForDelegate(enumDevicesHandler);
	//EnumDevicesCallback enumDevicesCallback = static_cast<EnumDevicesCallback>(intPtr.ToPointer());

	//try
	//{
	//	
	//}
	//finally
	//{
	//	if (enumDevicesHandle.IsAllocated)
	//{
	//		enumDevicesHandle.Free();
	//}
	//}

	System::Collections::Generic::List<ManagedWrapper^>^ localStreams
			= gcnew System::Collections::Generic::List<ManagedWrapper^>();

	for (auto const& d : devices)
	{
		localStreams->Add(gcnew ManagedWrapper(gcnew String(d.c_str())));
	}

	//ManagedWrapper^ stream = gcnew ManagedWrapper("Integrated Camera");
	//localStreams->Add(stream);

	return localStreams;

}

ManagedWrapper::~ManagedWrapper()
{
	playerPtr_->Uninitialize();

	delete playerPtr_;

	if (frameRecievedHandle_.IsAllocated)
	{
		frameRecievedHandle_.Free();
	}

	if (streamErrorHandle_.IsAllocated)
	{
		streamErrorHandle_.Free();
	}

	if (streamStoppedHandle_.IsAllocated)
	{
		streamStoppedHandle_.Free();
	}
}

Bitmap^ ManagedWrapper::ToBitmap(uint8_t *bmpPtr)
{
	try
	{
		BITMAPINFOHEADER *biHeaderPtr = reinterpret_cast<BITMAPINFOHEADER*>(bmpPtr);
		int32_t stride = biHeaderPtr->biWidth * (biHeaderPtr->biBitCount / 8);

		// The bits in the array are packed together, but each scan line must be
		// padded with zeros to end on a LONG data-type boundary.
		int32_t padding = stride % 4 > 0 ? 4 - stride % 4 : 0;
		stride += padding;

		IntPtr scan0(bmpPtr + sizeof(BITMAPINFOHEADER));

		Bitmap^ image = gcnew Bitmap(biHeaderPtr->biWidth, biHeaderPtr->biHeight, stride,
			System::Drawing::Imaging::PixelFormat::Format24bppRgb, scan0);
		image->RotateFlip(System::Drawing::RotateFlipType::RotateNoneFlipY);
		return image;
	}
	finally
	{
		if (bmpPtr != nullptr)
		{
			delete[] bmpPtr;
		}
	}
}

void ManagedWrapper::RaiseFrameRecievedEvent(uint8_t *bmpPtr)
{
	Bitmap^ frame = ToBitmap(bmpPtr);
	FrameRecieved(frame);
}

void ManagedWrapper::RaiseStreamFailedEvent(const char* error)
{
	StreamFailed(gcnew String(error));
}

void ManagedWrapper::RaiseStreamStoppedEvent()
{
	StreamStopped();
}