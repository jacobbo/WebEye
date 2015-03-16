using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Windows;
using Size = System.Drawing.Size;

namespace WebEye
{
    /// <summary>
    /// Interaction logic for WebCameraControl.xaml
    /// </summary>
    public partial class WebCameraControl
    {
        public WebCameraControl()
        {
            InitializeComponent();

            Dispatcher.ShutdownStarted += OnShutdownStarted; 
        }

        private DirectShowUtilities _utilities;

        private DirectShowUtilities Utilities
        {
            get { return _utilities ?? (_utilities = new DirectShowUtilities()); }
        }

        private readonly List<WebCameraId> _captureDevices = new List<WebCameraId>();
        private void SaveVideoDevice(ref DirectShowUtilities.VideoInputDeviceInfo info)
        {
            _captureDevices.Add(new WebCameraId(info));
        }

        /// <summary>
        /// Gets a list of available video capture devices.
        /// </summary>
        /// <exception cref="Win32Exception">Failed to load the DirectShow utilities dll.</exception>
        public IEnumerable<WebCameraId> GetVideoCaptureDevices()
        {
            _captureDevices.Clear();
            Utilities.EnumVideoInputDevices(SaveVideoDevice);

            return new List<WebCameraId>(_captureDevices);
        }

        /// <summary>
        /// Initializes a capture graph.
        /// </summary>
        /// <exception cref="Win32Exception">Failed to load the DirectShow utilities dll.</exception>
        /// <exception cref="DirectShowException">Failed to setup a capture graph.</exception>
        private void InitializeCaptureGraph()
        {
            Utilities.BuildCaptureGraph();
            Utilities.AddRenderFilter(_videoWindow.Handle);
        }

        private static readonly DependencyPropertyKey IsCapturingPropertyKey
            = DependencyProperty.RegisterReadOnly("IsCapturing", typeof(Boolean), typeof(WebCameraControl),
                new FrameworkPropertyMetadata(false));

        public static readonly DependencyProperty IsCapturingProperty
            = IsCapturingPropertyKey.DependencyProperty;

        /// <summary>
        /// Gets a value indicating whether the control is capturing a video stream.
        /// </summary>
        [Browsable(false)]
        public Boolean IsCapturing
        {
            get { return (Boolean)GetValue(IsCapturingProperty); }
            private set { SetValue(IsCapturingPropertyKey, value); }
        }

        private Boolean _captureGraphInitialized;
        private WebCameraId _currentCamera;

        /// <summary>
        /// Starts a capture.
        /// </summary>
        /// <param name="camera">The camera to capture from.</param>
        /// <exception cref="ArgumentNullException">A null reference is passed as an argument.</exception>
        /// <exception cref="Win32Exception">Failed to load the DirectShow utilities dll.</exception>
        /// <exception cref="DirectShowException">Failed to run a video capture graph.</exception>
        public void StartCapture(WebCameraId camera)
        {
            if (camera == null)
            {
                throw new ArgumentNullException();
            }

            if (!_captureGraphInitialized)
            {
                InitializeCaptureGraph();

                _captureGraphInitialized = true;
            }

            if (IsCapturing)
            {
                if (_currentCamera == camera)
                {
                    return;
                }

                StopCapture();
            }

            if (_currentCamera != null)
            {
                Utilities.ResetCaptureGraph();
                _currentCamera = null;
            }

            Utilities.AddCaptureFilter(camera.DevicePath);
            _currentCamera = camera;

            try
            {
                Utilities.Start();
                IsCapturing = true;
            }
            catch (DirectShowException)
            {
                Utilities.ResetCaptureGraph();
                _currentCamera = null;
                throw;
            }
        }

        /// <summary>
        /// Retrieves the unstretched image being captured.
        /// </summary>
        /// <returns>The current image.</returns>
        /// <exception cref="InvalidOperationException">The control is not capturing a video stream.</exception>
        /// <exception cref="DirectShowException">Failed to get the current image.</exception>
        public Bitmap GetCurrentImage()
        {
            if (!IsCapturing)
            {
                throw new InvalidOperationException();
            }

            return Utilities.GetCurrentImage();
        }

        /// <summary>
        /// Gets the unstretched video size.
        /// </summary>
        [Browsable(false)]
        public Size VideoSize
        {
            get { return IsCapturing ? Utilities.GetVideoSize() : new Size(0, 0); }
        }

        /// <summary>
        /// Stops a capture.
        /// </summary>
        /// <exception cref="InvalidOperationException">The control is not capturing a video stream.</exception>
        /// <exception cref="DirectShowException">Failed to stop a video capture graph.</exception>
        public void StopCapture()
        {
            if (!IsCapturing)
            {
                throw new InvalidOperationException();
            }

            Utilities.Stop();
            IsCapturing = false;

            Utilities.ResetCaptureGraph();
            _currentCamera = null;
        }

        private void OnShutdownStarted(object sender, EventArgs eventArgs)
        {
            if (_utilities != null)
            {
                if (IsCapturing)
                {
                    StopCapture();
                }

                _utilities.DestroyCaptureGraph();
                _utilities.Dispose();
            }
        }

        protected override void OnContentChanged(object oldContent, object newContent)
        {
            if (oldContent != null)
            {
                throw new InvalidOperationException();
            }
        }
    }
}
