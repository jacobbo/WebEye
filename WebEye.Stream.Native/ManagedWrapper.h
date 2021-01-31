#pragma once

#include "Player.h"

namespace WebEye
{
    public delegate void FrameRecievedHandler(System::Drawing::Bitmap^ bmp);
    public delegate void StreamFailedHandler(System::String^ error);
    public delegate void StreamStoppedHandler();

    delegate void FramePtrRecievedHandler(uint8_t *bmpPtr);
    delegate void StreamErrorHandler(const char* error);

    public ref class ManagedWrapper sealed
    {
    public:
        static ManagedWrapper^ FromUri(System::Uri^ uri, System::TimeSpan^ connectionTimeout,
            System::TimeSpan^ streamTimeout, WebEye::RtspTransport transport, WebEye::RtspFlags flags);

        static System::Collections::Generic::IEnumerable<ManagedWrapper^>^ GetLocalStreams();

        property System::String^ Name { System::String^ get(); }

        /// <summary>
        /// Asynchronously plays a stream.
        /// </summary>
        void Start();

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

        event FrameRecievedHandler^ FrameRecieved;
        event StreamFailedHandler^ StreamFailed;
        event StreamStoppedHandler^ StreamStopped;

        ~ManagedWrapper();

    private:
        ManagedWrapper();

        ManagedWrapper(System::String^ deviceName);

        ManagedWrapper(System::Uri^ uri, System::TimeSpan^ connectionTimeout,
            System::TimeSpan^ streamTimeout, WebEye::RtspTransport transport, WebEye::RtspFlags flags);

        void RaiseFrameRecievedEvent(uint8_t *bmpPtr);

        void RaiseStreamFailedEvent(const char* error);

        void RaiseStreamStoppedEvent();

        System::Drawing::Bitmap^ ToBitmap(uint8_t *bmpPtr);

        FFmpeg::Facade::Player *playerPtr_;

        FrameRecievedHandler^ frameRecievedHandler_;
        StreamFailedHandler^ streamFailedHandler_;
        StreamStoppedHandler^ streamStoppedHandler_;

        System::Runtime::InteropServices::GCHandle frameRecievedHandle_;
        System::Runtime::InteropServices::GCHandle streamErrorHandle_;
        System::Runtime::InteropServices::GCHandle streamStoppedHandle_;

        System::String^ url_;
        int connectionTimeout_;
        int streamTimeout_;
        RtspTransport transport_;
        RtspFlags flags_;

        System::String^ deviceName_;
    };
}