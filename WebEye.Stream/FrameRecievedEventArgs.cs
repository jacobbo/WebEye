using System;
using System.Drawing;

namespace WebEye
{
    public class FrameRecievedEventArgs : EventArgs
    {
        public FrameRecievedEventArgs(Bitmap frame)
        {
            Frame = frame;
        }

        public Bitmap Frame { get; }
    }
}
