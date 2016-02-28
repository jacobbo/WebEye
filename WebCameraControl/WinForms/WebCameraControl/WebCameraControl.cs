namespace WebEye.Controls.WinForms.WebCameraControl
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Drawing;
    using System.Windows.Forms;

    public sealed partial class WebCameraControl : UserControl
    {
        /// <summary>
        /// Initializes a new instance of the WebCameraControl class.
        /// </summary>
        public WebCameraControl()
        {
            this.InitializeComponent();
        }

        private DirectShowProxy _proxy;

        private DirectShowProxy Proxy
        {
            get { return this._proxy ?? (this._proxy = new DirectShowProxy()); }
        }

        private readonly List<WebCameraId> _captureDevices = new List<WebCameraId>();
        private void SaveVideoDevice(ref DirectShowProxy.VideoInputDeviceInfo info)
        {
            this._captureDevices.Add(new WebCameraId(info));
        }

        /// <summary>
        /// Gets a list of available video capture devices.
        /// </summary>
        /// <exception cref="Win32Exception">Failed to load the DirectShow utilities dll.</exception>
        public IEnumerable<WebCameraId> GetVideoCaptureDevices()
        {
            this._captureDevices.Clear();
            this.Proxy.EnumVideoInputDevices(this.SaveVideoDevice);

            return new List<WebCameraId>(this._captureDevices);
        }

        /// <summary>
        /// Initializes a capture graph.
        /// </summary>
        /// <exception cref="Win32Exception">Failed to load the DirectShow utilities dll.</exception>
        /// <exception cref="DirectShowException">Failed to setup a capture graph.</exception>
        private void InitializeCaptureGraph()
        {
            this.Proxy.BuildCaptureGraph();
            this.Proxy.AddRenderFilter(this.Handle);
        }

        private Boolean _isCapturing;

        /// <summary>
        /// Gets a value indicating whether the control is capturing a video stream.
        /// </summary>
        [Browsable(false)]
        public Boolean IsCapturing { get { return this._isCapturing; } }

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

            if (!this._captureGraphInitialized)
            {
                this.InitializeCaptureGraph();

                this._captureGraphInitialized = true;
            }

            if (this._isCapturing)
            {
                if (this._currentCamera == camera)
                {
                    return;
                }

                this.StopCapture();
            }

            if (this._currentCamera != null)
            {
                this.Proxy.ResetCaptureGraph();
                this._currentCamera = null;
            }

            this.Proxy.AddCaptureFilter(camera.DevicePath);
            this._currentCamera = camera;

            try
            {
                this.Proxy.Start();
                this._isCapturing = true;
            }
            catch (DirectShowException)
            {
                this.Proxy.ResetCaptureGraph();
                this._currentCamera = null;
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
            if (!this._isCapturing)
            {
                throw new InvalidOperationException();
            }

            return this.Proxy.GetCurrentImage();
        }

        /// <summary>
        /// Gets the unstretched video size.
        /// </summary>
        [Browsable(false)]
        public Size VideoSize
        {
            get { return this._isCapturing ? this.Proxy.GetVideoSize() : new Size(0, 0); }
        }

        /// <summary>
        /// Stops a capture.
        /// </summary>
        /// <exception cref="InvalidOperationException">The control is not capturing a video stream.</exception>
        /// <exception cref="DirectShowException">Failed to stop a video capture graph.</exception>
        public void StopCapture()
        {
            if (!this._isCapturing)
            {
                throw new InvalidOperationException();
            }

            this.Proxy.Stop();
            this._isCapturing = false;

            this.Proxy.ResetCaptureGraph();
            this._currentCamera = null;
        }

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (this._proxy != null))
            {
                if (this._isCapturing)
                {
                    this.StopCapture();
                }

                this.Proxy.DestroyCaptureGraph();
                this.Proxy.Dispose();
            }

            if (disposing && (this.components != null))
            {
                this.components.Dispose();
            }

            base.Dispose(disposing);
        }
    }
}
