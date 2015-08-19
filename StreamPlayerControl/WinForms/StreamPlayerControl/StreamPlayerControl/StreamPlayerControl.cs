using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace WebEye
{
    using System.Runtime.InteropServices;

    /// <summary>
    /// The stream player control.
    /// </summary>
    public partial class StreamPlayerControl: UserControl
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="StreamPlayerControl"/> class.
        /// </summary>
        public StreamPlayerControl()
        {
            InitializeComponent();
        }

        private StreamPlayerProxy _player;

        private StreamPlayerProxy Player
        {
            get
            {
                if (_player == null)
                {
                    _player = CreateAndInitializePlayer();
                }

                return _player;
            }
        }

        /// <summary>
        /// Asynchronously plays a stream.
        /// </summary>
        /// <param name="uri">The uri of a stream to play.</param>
        /// <exception cref="ArgumentException">An invalid string is passed as an argument.</exception>
        /// <exception cref="Win32Exception">Failed to load the FFmpeg facade dll.</exception>
        /// <exception cref="StreamPlayerException">Failed to play the stream.</exception>
        public void StartPlay(Uri uri)
        {
            if (IsPlaying)
            {
                Stop();
            }

            Player.StartPlay(uri.ToString());
        }

        /// <summary>
        /// Retrieves the image being played.
        /// </summary>
        /// <returns>The current image.</returns>
        /// <exception cref="InvalidOperationException">The control is not playing a video stream.</exception>
        /// <exception cref="StreamPlayerException">Failed to get the current image.</exception>
        public Bitmap GetCurrentFrame()
        {
            if (!IsPlaying)
            {
                throw new InvalidOperationException();
            }

            return Player.GetCurrentFrame();
        }

        /// <summary>
        /// Stops a stream.
        /// </summary>
        /// <exception cref="InvalidOperationException">The control is not playing a stream.</exception>
        /// <exception cref="StreamPlayerException">Failed to stop a stream.</exception>
        public void Stop()
        {
            if (!IsPlaying)
            {
                throw new InvalidOperationException();
            }

            Player.Stop();

            IsPlaying = false;
        }

        /// <summary>
        /// Gets a value indicating whether the control is playing a video stream.
        /// </summary>
        [Browsable(false)]
        public Boolean IsPlaying { get; private set; }

        /// <summary>
        /// Gets the unstretched frame size.
        /// </summary>
        [Browsable(false)]
        public Size VideoSize
        {
            get { return IsPlaying ? Player.GetFrameSize() : new Size(0, 0); }
        }

        private bool _disposed;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (_disposed)
            {
                return;
            }

            if (disposing && (_player != null))
            {
                if (IsPlaying)
                {
                    Stop();
                }

                _player.Uninitialize();
                _player.Dispose();
            }

            if (disposing && (components != null))
            {
                components.Dispose();
            }

            _disposed = true;
            base.Dispose(disposing);
        }

        /// <summary>
        /// Specifies a set of values that are used when you start the player.
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        internal struct StreamPlayerParams
        {
            internal IntPtr window;
            internal IntPtr streamStartedCallback;
            internal IntPtr streamStoppedCallback;
            internal IntPtr streamFailedCallback;
        }

        [UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
        private delegate void CallbackDelegate();

        private Delegate _streamStartedCallback;
        private Delegate _streamStoppedCallback;
        private Delegate _streamFailedCallback;

        private StreamPlayerProxy CreateAndInitializePlayer()
        {
            var player = new StreamPlayerProxy();

            _streamStartedCallback = new CallbackDelegate(RaiseStreamStartedEvent);
            _streamStoppedCallback = new CallbackDelegate(RaiseStreamStoppedEvent);
            _streamFailedCallback = new CallbackDelegate(RaiseStreamFailedEvent);

            var playerParams = new StreamPlayerParams
            {
                window = Handle,
                streamStartedCallback = Marshal.GetFunctionPointerForDelegate(_streamStartedCallback),
                streamStoppedCallback = Marshal.GetFunctionPointerForDelegate(_streamStoppedCallback),
                streamFailedCallback = Marshal.GetFunctionPointerForDelegate(_streamFailedCallback)
            };

            player.Initialize(playerParams);

            return player;
        }

        /// <summary>
        /// Occurs when the first frame is read from a stream.
        /// </summary>
        public event EventHandler StreamStarted;

        private void RaiseStreamStartedEvent()
        {
            IsPlaying = true;

            if (StreamStarted != null)
            {
                StreamStarted(this, EventArgs.Empty);
            }
        }

        /// <summary>
        /// Occurs when there are no more frames to read from a stream.
        /// </summary>
        public event EventHandler StreamStopped;

        private void RaiseStreamStoppedEvent()
        {
            IsPlaying = false;

            if (StreamStopped != null)
            {
                StreamStopped(this, EventArgs.Empty);
            }
        }

        /// <summary>
        /// Occurs when the player fails to play a stream.
        /// </summary>
        public event EventHandler StreamFailed;

        private void RaiseStreamFailedEvent()
        {
            IsPlaying = false;

            if (StreamFailed != null)
            {
                StreamFailed(this, EventArgs.Empty);
            }
        }
    }
}
