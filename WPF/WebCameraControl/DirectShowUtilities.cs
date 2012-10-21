using System;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.ConstrainedExecution;
using System.Runtime.InteropServices;
using WebEye.Properties;

namespace WebEye
{
    public sealed class DirectShowException : Exception
    {
        internal DirectShowException(string message, Int32 hresult)
            : base(message, Marshal.GetExceptionForHR(hresult))
        {
        }
    }

    internal sealed class DirectShowUtilities : CriticalFinalizerObject, IDisposable
    {
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        internal struct VideoInputDeviceInfo
        {
            [MarshalAs(UnmanagedType.BStr)]
            internal String FriendlyName;

            [MarshalAs(UnmanagedType.BStr)]
            internal String DevicePath;
        }

        internal delegate void EnumVideoInputDevicesCallback(ref VideoInputDeviceInfo info);
        private delegate void EnumVideoInputDevicesDelegate(EnumVideoInputDevicesCallback callback);
        private EnumVideoInputDevicesDelegate _enumVideoInputDevices;

        private delegate Int32 BuildCaptureGraphDelegate();
        private BuildCaptureGraphDelegate _buildCaptureGraph;

        private delegate Int32 AddRenderFilterDelegate(IntPtr hWnd);
        private AddRenderFilterDelegate _addRenderFilter;

        private delegate Int32 AddCaptureFilterDelegate([MarshalAs(UnmanagedType.BStr)]String devicePath);
        private AddCaptureFilterDelegate _addCaptureFilter;

        private delegate Int32 ResetCaptureGraphDelegate();
        private ResetCaptureGraphDelegate _resetCaptureGraph;

        private delegate Int32 StartDelegate();
        private StartDelegate _start;

        private delegate Int32 GetCurrentImageDelegate([Out] out IntPtr dibPtr);
        private GetCurrentImageDelegate _getCurrentImage;

        private delegate Int32 GetVideoSizeDelegate(out Int32 width, out Int32 height);
        private GetVideoSizeDelegate _getVideoSize;

        private delegate Int32 StopDelegate();
        private StopDelegate _stop;

        private delegate void DestroyCaptureGraphDelegate();
        private DestroyCaptureGraphDelegate _destroyCaptureGraph;

        private String _dllFile = string.Empty;
        private IntPtr _hDll = IntPtr.Zero;

        [DllImport("kernel32", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern IntPtr LoadLibrary(String lpFileName);

        /// <summary>
        /// Extracts the directshow utils dll from resources to the temp file and loads it.
        /// </summary>
        /// <exception cref="Win32Exception">Failed to load the utilities dll.</exception>
        private void LoadDll()
        {
            _dllFile = Path.GetTempFileName();
            using (var stream = new FileStream(_dllFile, FileMode.Create, FileAccess.Write))
            {
                using (var writer = new BinaryWriter(stream))
                {
                    writer.Write(Resources.DSUtilsDLL);
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
            IntPtr pProcPtr = GetProcAddress(hDll, "EnumVideoInputDevices");
            _enumVideoInputDevices =
                (EnumVideoInputDevicesDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr, typeof(EnumVideoInputDevicesDelegate));

            pProcPtr = GetProcAddress(hDll, "BuildCaptureGraph");
            _buildCaptureGraph =
                (BuildCaptureGraphDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr, typeof(BuildCaptureGraphDelegate));

            pProcPtr = GetProcAddress(hDll, "AddRenderFilter");
            _addRenderFilter =
                (AddRenderFilterDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr, typeof(AddRenderFilterDelegate));

            pProcPtr = GetProcAddress(hDll, "AddCaptureFilter");
            _addCaptureFilter =
                (AddCaptureFilterDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr, typeof(AddCaptureFilterDelegate));

            pProcPtr = GetProcAddress(hDll, "ResetCaptureGraph");
            _resetCaptureGraph =
                (ResetCaptureGraphDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr, typeof(ResetCaptureGraphDelegate));

            pProcPtr = GetProcAddress(hDll, "Start");
            _start = (StartDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr, typeof(StartDelegate));

            pProcPtr = GetProcAddress(hDll, "GetCurrentImage");
            _getCurrentImage =
                (GetCurrentImageDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr, typeof(GetCurrentImageDelegate));

            pProcPtr = GetProcAddress(hDll, "GetVideoSize");
            _getVideoSize =
                (GetVideoSizeDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr, typeof(GetVideoSizeDelegate));

            pProcPtr = GetProcAddress(hDll, "Stop");
            _stop = (StopDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr, typeof(StopDelegate));

            pProcPtr = GetProcAddress(hDll, "DestroyCaptureGraph");
            _destroyCaptureGraph =
                (DestroyCaptureGraphDelegate)Marshal.GetDelegateForFunctionPointer(pProcPtr, typeof(DestroyCaptureGraphDelegate));            
        }

        /// <summary>
        /// Initializes a new instance of the DirectShowUtilities class.
        /// </summary>
        /// <exception cref="Win32Exception">Failed to load the utilities dll.</exception>
        internal DirectShowUtilities()
        {
            LoadDll();
            BindToDll(_hDll);
        }

        /// <summary>
        /// Enumerates video input devices in a system.
        /// </summary>
        /// <param name="callback">A callback method.</param>
        internal void EnumVideoInputDevices(EnumVideoInputDevicesCallback callback)
        {
            _enumVideoInputDevices(callback);
        }

        /// <summary>
        /// Throws the DirectShowException exception for the result.
        /// </summary>
        /// <param name="hresult">The result to check.</param>
        /// <param name="message">The exception message.</param>
        private static void ThrowExceptionForResult(Int32 hresult, String message)
        {
            if (hresult < 0)
            {
                throw new DirectShowException(message, hresult);
            }
        }

        /// <summary>
        /// Builds a video capture graph.
        /// </summary>
        internal void BuildCaptureGraph()
        {
            ThrowExceptionForResult(_buildCaptureGraph(), "Failed to build a video capture graph.");
        }

        /// <summary>
        /// Adds a renderer filter to a video capture graph, which renders a video stream within a container window.
        /// </summary>
        /// <param name="hWnd">A container window that video should be clipped to.</param>
        internal void AddRenderFilter(IntPtr hWnd)
        {
            ThrowExceptionForResult(_addRenderFilter(hWnd), "Failed to setup a render filter.");
        }

        /// <summary>
        /// Adds a video stream source to a video capture graph.
        /// </summary>
        /// <param name="devicePath">A device path of a video capture filter to add.</param>
        internal void AddCaptureFilter(String devicePath)
        {
            ThrowExceptionForResult(_addCaptureFilter(devicePath), "Failed to add a video capture filter.");
        }

        /// <summary>
        /// Removes a video stream source from a video capture graph.
        /// </summary>
        internal void ResetCaptureGraph()
        {
            ThrowExceptionForResult(_resetCaptureGraph(), "Failed to reset a video capture graph.");
        }

        /// <summary>
        /// Runs all the filters in a video capture graph. While the graph is running,
        /// data moves through the graph and is rendered. 
        /// </summary>
        internal void Start()
        {
            ThrowExceptionForResult(_start(), "Failed to run a capture graph.");
        }

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
        /// Retrieves the current image being displayed by the renderer filter.
        /// </summary>
        /// <returns>The current image being displayed by the renderer filter.</returns>
        internal Bitmap GetCurrentImage()
        {
            IntPtr dibPtr;
            ThrowExceptionForResult(_getCurrentImage(out dibPtr), "Failed to get the current image.");

            try
            {
                BITMAPINFOHEADER biHeader = (BITMAPINFOHEADER)Marshal.PtrToStructure(dibPtr, typeof(BITMAPINFOHEADER));
                Int32 stride = biHeader.biWidth * (biHeader.biBitCount / 8);

                PixelFormat pixelFormat = PixelFormat.Undefined;
                switch (biHeader.biBitCount)
                {
                    case 1:
                        pixelFormat = PixelFormat.Format1bppIndexed;
                        break;
                    case 4:
                        pixelFormat = PixelFormat.Format4bppIndexed;
                        break;
                    case 8:
                        pixelFormat = PixelFormat.Format8bppIndexed;
                        break;
                    case 16:
                        pixelFormat = PixelFormat.Format16bppRgb555;
                        break;
                    case 24:
                        pixelFormat = PixelFormat.Format24bppRgb;
                        break;
                    case 32:
                        pixelFormat = PixelFormat.Format32bppRgb;
                        break;
                }

                Bitmap image = new Bitmap(biHeader.biWidth, biHeader.biHeight, stride,
                    pixelFormat, (IntPtr)(dibPtr.ToInt64() + Marshal.SizeOf(biHeader)));
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
        /// Retrieves the unstretched video size.
        /// </summary>
        /// <returns>The unstretched video size.</returns>
        internal Size GetVideoSize()
        {
            Int32 width, height;
            ThrowExceptionForResult(_getVideoSize(out width, out height), "Failed to get the video size.");

            return new Size(width, height);
        }

        /// <summary>
        /// Stops all the filters in a video filter graph.
        /// </summary>
        internal void Stop()
        {
            ThrowExceptionForResult(_stop(), "Failed to stop a video capture graph.");
        }

        /// <summary>
        /// Destroys a video capture graph.
        /// </summary>
        internal void DestroyCaptureGraph()
        {
            _destroyCaptureGraph();
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern Boolean FreeLibrary(IntPtr hModule);

        private void Dispose(Boolean disposing)
        {
            // If you need thread safety, use a lock around these 
            // operations, as well as in your methods that use the resource.
            if (disposing)
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
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        ~DirectShowUtilities()
        {
            Dispose(false);
        }
    }
}
