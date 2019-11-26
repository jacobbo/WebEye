using System;

namespace WebEye
{
    public class StreamFailedEventArgs : EventArgs
    {
        public StreamFailedEventArgs(String error)
        {
            Error = error;
        }

        public String Error { get; }
    }
}
