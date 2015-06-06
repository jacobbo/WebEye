using System;
using System.ComponentModel;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Windows;
using Size = System.Windows.Size;

namespace WebEye
{
    /// <summary>
    /// Interaction logic for StreamPlayerControl.xaml
    /// </summary>
    public partial class StreamPlayerControl
    {
        public StreamPlayerControl()
        {
            InitializeComponent();

            Dispatcher.ShutdownStarted += HandleShutdownStarted; 
        }

        private StreamPlayerProxy _player;

        private StreamPlayerProxy Player
        {
            get
            {
                if (_player == null)
                {
                    _player = new StreamPlayerProxy();

                    var playFailedCallback = (Delegate)new ManagedTypeSignature(PlayFailedCallback);
                    //IntPtr nativeThunk = Marshal.GetFunctionPointerForDelegate(playFailedCallback);

                    var playSucceededCallback = (Delegate)new ManagedTypeSignature(PlaySucceededCallback);
                    //IntPtr nativeThunk = Marshal.GetFunctionPointerForDelegate(playFailedCallback);

                    var playerParams = new StreamPlayerParams
                    {
                        window = _videoWindow.Handle,
                        playFailedCallback = Marshal.GetFunctionPointerForDelegate(playFailedCallback),
                        playSucceededCallback = Marshal.GetFunctionPointerForDelegate(playSucceededCallback)
                    };

                    _player.Initialize(playerParams);
                }

                return _player;
            }
        }

        /// <summary>
        /// Asynchronously plays a stream.
        /// </summary>
        /// <param name="url">The url of a stream to play.</param>
        /// <exception cref="ArgumentException">An invalid string is passed as an argument.</exception>
        /// <exception cref="Win32Exception">Failed to load the FFmpeg facade dll.</exception>
        /// <exception cref="StreamPlayerException">Failed to play the stream.</exception>
        public void StartPlay(String url)
        {
            if (string.IsNullOrWhiteSpace(url))
            {
                throw new ArgumentException();
            }

            if (IsPlaying)
            {
                Stop();
            }

            Player.StartPlay(url);
            //Player.Play();

            //IsPlaying = true;
        }

        /// <summary>
        /// Retrieves the unstretched image being played.
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

        private static readonly DependencyPropertyKey IsPlayingPropertyKey =
            DependencyProperty.RegisterReadOnly("IsPlaying", typeof(Boolean), typeof(StreamPlayerControl),
        new FrameworkPropertyMetadata(false));

        public static readonly DependencyProperty IsPlayingProperty
            = IsPlayingPropertyKey.DependencyProperty;

        /// <summary>
        /// Gets a value indicating whether the control is playing a video stream.
        /// </summary>
        [Browsable(false)]
        public Boolean IsPlaying
        {
            get { return (Boolean)GetValue(IsPlayingProperty); }
            private set { SetValue(IsPlayingPropertyKey, value); }
        }

        /// <summary>
        /// Gets the unstretched frame size.
        /// </summary>
        [Browsable(false)]
        public Size VideoSize
        {
            get { return IsPlaying ? Player.GetFrameSize() : new Size(0, 0); }
        }

        private void HandleShutdownStarted(object sender, EventArgs eventArgs)
        {
            if (_player != null)
            {
                if (IsPlaying)
                {
                    Stop();
                }

                _player.Uninitialize();
                _player.Dispose();
            }
        }

        protected override void OnContentChanged(object oldContent, object newContent)
        {
            if (oldContent != null)
            {
                throw new InvalidOperationException();
            }
        }

        [UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
        delegate void ManagedTypeSignature();

        [StructLayout(LayoutKind.Sequential)]
        internal struct StreamPlayerParams
        {
            public IntPtr window;
            public IntPtr playFailedCallback;
            public IntPtr playSucceededCallback;
        }

        private void PlayFailedCallback()
        {
            RaisePlayFailedEvent();
        }

        private void PlaySucceededCallback()
        {
            RaisePlaySucceededEvent();
        }

        public static readonly RoutedEvent PlayFailedEvent =
            EventManager.RegisterRoutedEvent("PlayFailed", RoutingStrategy.Bubble,
            typeof(RoutedEventHandler), typeof(StreamPlayerControl));

        public event RoutedEventHandler PlayFailed
        {
            add { AddHandler(PlayFailedEvent, value); }
            remove { RemoveHandler(PlayFailedEvent, value); }
        }

        private void RaisePlayFailedEvent()
        {
            RaiseEvent(new RoutedEventArgs(PlayFailedEvent));
        }

        public static readonly RoutedEvent PlaySucceededEvent =
            EventManager.RegisterRoutedEvent("PlaySucceeded", RoutingStrategy.Bubble,
            typeof(RoutedEventHandler), typeof(StreamPlayerControl));

        public event RoutedEventHandler PlaySucceeded
        {
            add { AddHandler(PlaySucceededEvent, value); }
            remove { RemoveHandler(PlaySucceededEvent, value); }
        }

        private void RaisePlaySucceededEvent()
        {
            IsPlaying = true;
            RaiseEvent(new RoutedEventArgs(PlaySucceededEvent));
        }
    }
}
