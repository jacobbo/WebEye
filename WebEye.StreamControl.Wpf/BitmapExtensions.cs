using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace WebEye.StreamControl.Wpf
{
    public static class BitmapExtensions
    {
        public static WriteableBitmap ToWriteableBitmap(this Bitmap source)
        {
            var bitmapData = source.LockBits(new Rectangle(0, 0, source.Width, source.Height),
                ImageLockMode.ReadOnly, source.PixelFormat);

            try
            {
                var bitmapSource = BitmapSource.Create(
                    bitmapData.Width, bitmapData.Height,
                    source.HorizontalResolution, source.VerticalResolution,
                    PixelFormats.Bgr24, null,
                    bitmapData.Scan0, bitmapData.Stride * bitmapData.Height, bitmapData.Stride);

                return new WriteableBitmap(bitmapSource);
            }
            finally
            {
                source.UnlockBits(bitmapData);
            }
        }

        [DllImport("kernel32.dll", EntryPoint = "CopyMemory", SetLastError = false)]
        private static extern void CopyMemory(IntPtr dest, IntPtr src, uint count);

        public static void UpdateWith(this WriteableBitmap target, Bitmap source)
        {
            var bitmapData = source.LockBits(new Rectangle(0, 0, source.Width, source.Height),
                ImageLockMode.ReadOnly, source.PixelFormat);
            target.Lock();

            try
            {
                CopyMemory(target.BackBuffer, bitmapData.Scan0,
                    (UInt32)(target.BackBufferStride * bitmapData.Height));
                target.AddDirtyRect(new Int32Rect(0, 0, bitmapData.Width, bitmapData.Height));
            }
            finally
            {
                target.Unlock();
                source.UnlockBits(bitmapData);
            }
        }
    }
}