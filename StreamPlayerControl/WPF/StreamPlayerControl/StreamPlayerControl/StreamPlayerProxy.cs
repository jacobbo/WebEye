﻿using System;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using WebEye.Properties;
using Size = System.Windows.Size;

namespace WebEye
{
    public sealed class StreamPlayerException : Exception
    {
        internal StreamPlayerException(string message)
            : base(message) { }
    }

    internal sealed class StreamPlayerProxy : IDisposable
    {
        /// <summary>
        /// Initializes a new instance of the StreamPlayerProxy class.
        /// </summary>
        /// <exception cref="Win32Exception">Failed to load the utilities dll.</exception>
        internal StreamPlayerProxy()
        {
            LoadDll();
            BindToDll(_hDll);
        }

        /// <summary>
        /// Initializes the player.
        /// </summary>
        /// <param name="window">A container window that video should be clipped to.</param>
        internal void Initialize(IntPtr window)
        {
            if (_initialize(window) != 0)
            {
                throw new StreamPlayerException("Failed to initialize the player.");
            }
        }

        /// <summary>
        /// Opens a stream.
        /// </summary>
        /// <param name="url">The url of a stream to open.</param>
        internal void Open(String url)
        {
            if (_open(url) != 0)
            {
                throw new StreamPlayerException("Failed to open the stream.");
            }
        }

        /// <summary>
        /// Plays the stream opened by the Open method.
        /// </summary>
        /// <param name="awaitStream">If set the method awaits for video stream before exiting</param>
        internal void Play(bool awaitStream=false)
        {
            if (_play() != 0)
            {
                throw new StreamPlayerException("Failed to play the stream.");
            }

            if (awaitStream)
            {
                bool success = false;
                while (!success)
                {
                    try
                    {
                        Thread.Sleep(100);
                        GetCurrentFrame();
                        success = true;
                    }
                    catch
                    { }
                }
            }
        }

        /// <summary>
        /// Stops a stream.
        /// </summary>
        internal void Stop()
        {
            if (_stop() != 0)
            {
                throw new StreamPlayerException("Failed to stop the stream.");
            }
        }

        /// <summary>
        /// Uninitializes the player.
        /// </summary>
        internal void Uninitialize()
        {
            if (_uninitialize() != 0)
            {
                throw new StreamPlayerException("Failed to uninitialize the player.");
            }
        }

        /// <summary>
        /// The BITMAPINFOHEADER structure contains information about the dimensions and color format of a device independent bitmap.
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        private struct BITMAPINFOHEADER
        {
            public UInt32 biSize;
            public Int32 biWidth;
            public Int32 biHeight;
            public UInt16 biPlanes;
            public UInt16 biBitCount;
            public UInt32 biCompression;
            public UInt32 biSizeImage;
            public Int32 biXPelsPerMeter;
            public Int32 biYPelsPerMeter;
            public UInt32 biClrUsed;
            public UInt32 biClrImportant;
        }

        /// <summary>
        /// Retrieves the current frame being displayed by the player.
        /// </summary>
        /// <returns>The current frame being displayed by the player.</returns>
        internal Bitmap GetCurrentFrame()
        {
            IntPtr dibPtr;
            if (_getCurrentFrame(out dibPtr) != 0)
            {
                throw new StreamPlayerException("Failed to get the current image.");
            }

            try
            {
                BITMAPINFOHEADER biHeader = (BITMAPINFOHEADER)Marshal.PtrToStructure(dibPtr, typeof(BITMAPINFOHEADER));
                Int32 stride = biHeader.biWidth * (biHeader.biBitCount / 8);

                // The bits in the array are packed together, but each scan line must be
                // padded with zeros to end on a LONG data-type boundary.
                Int32 padding = stride % 4 > 0 ? 4 - stride % 4 : 0;
                stride += padding;

                Bitmap image = new Bitmap(biHeader.biWidth, biHeader.biHeight, stride,
                    PixelFormat.Format24bppRgb, (IntPtr)(dibPtr.ToInt64() + Marshal.SizeOf(biHeader)));
                image.RotateFlip(RotateFlipType.RotateNoneFlipY);

                return image;
            }
            finally
            {
                if (dibPtr != IntPtr.Zero)
                {
                    Marshal.FreeCoTaskMem(dibPtr);
                }
            }
        }

        /// <summary>
        /// Retrieves the unstretched frame size.
        /// </summary>
        /// <returns>The unstretched frame size.</returns>
        internal Size GetFrameSize()
        {
            Int32 width, height;
            if (_getFrameSize(out width, out height) != 0)
            {
                throw new StreamPlayerException("Failed to get the frame size.");
            }

            return new Size(width, height);
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern Boolean FreeLibrary(IntPtr hModule);
        public void Dispose()
        {
            if (_hDll != IntPtr.Zero)
            {
                FreeLibrary(_hDll);
            }

            if (File.Exists(_dllFile))
            {
                File.Delete(_dllFile);
            }
        }

        [DllImport("kernel32", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern IntPtr LoadLibrary(String lpFileName);

        /// <summary>
        /// Extracts the FFmpeg facade dll from resources to a temp file and loads it.
        /// </summary>
        /// <exception cref="Win32Exception">Failed to load the FFmpeg facade dll.</exception>
        private void LoadDll()
        {
            _dllFile = Path.GetTempFileName();
            using (var stream = new FileStream(_dllFile, FileMode.Create, FileAccess.Write))
            {
                using (var writer = new BinaryWriter(stream))
                {
                    writer.Write(Resources.StreamPlayer);
                }
            }

            _hDll = LoadLibrary(_dllFile);
            if (_hDll == IntPtr.Zero)
            {
                throw new Win32Exception(Marshal.GetLastWin32Error());
            }
        }

        [DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        private static extern IntPtr GetProcAddress(IntPtr hModule, String procName);

        /// <summary>
        /// Binds the class instance methods to the dll functions.
        /// </summary>
        /// <param name="hDll">A dll to bind to.</param>
        private void BindToDll(IntPtr hDll)
        {
            IntPtr pProcPtr = GetProcAddress(hDll, "Initialize");
            _initialize =
                (InitializeDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr, typeof(InitializeDelegate));

            pProcPtr = GetProcAddress(hDll, "Open");
            _open =
                (OpenDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr, typeof(OpenDelegate));

            pProcPtr = GetProcAddress(hDll, "Play");
            _play =
                (PlayDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr, typeof(PlayDelegate));

            pProcPtr = GetProcAddress(hDll, "GetCurrentFrame");
            _getCurrentFrame =
                (GetCurrentFrameDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr,
                typeof(GetCurrentFrameDelegate));

            pProcPtr = GetProcAddress(hDll, "GetFrameSize");
            _getFrameSize =
                (GetFrameSizeDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr,
                typeof(GetFrameSizeDelegate));

            pProcPtr = GetProcAddress(hDll, "Stop");
            _stop =
                (StopDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr, typeof(StopDelegate));

            pProcPtr = GetProcAddress(hDll, "Uninitialize");
            _uninitialize = (UninitializeDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr, typeof(UninitializeDelegate));
        }

        private delegate Int32 InitializeDelegate(IntPtr hWnd);
        private InitializeDelegate _initialize;

        private delegate Int32 OpenDelegate([MarshalAs(UnmanagedType.LPStr)]String url);
        private OpenDelegate _open;

        private delegate Int32 PlayDelegate();
        private PlayDelegate _play;

        private delegate Int32 GetCurrentFrameDelegate([Out] out IntPtr dibPtr);
        private GetCurrentFrameDelegate _getCurrentFrame;

        private delegate Int32 GetFrameSizeDelegate(out Int32 width, out Int32 height);
        private GetFrameSizeDelegate _getFrameSize;

        private delegate Int32 StopDelegate();
        private StopDelegate _stop;

        private delegate Int32 UninitializeDelegate();
        private UninitializeDelegate _uninitialize;

        private String _dllFile = string.Empty;
        private IntPtr _hDll = IntPtr.Zero;
    }
}

