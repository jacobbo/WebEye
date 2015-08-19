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
        delegate void CallbackDelegate();
        
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
                window = _videoWindow.Handle,
                streamStartedCallback = Marshal.GetFunctionPointerForDelegate(_streamStartedCallback),
                streamStoppedCallback = Marshal.GetFunctionPointerForDelegate(_streamStoppedCallback),
                streamFailedCallback = Marshal.GetFunctionPointerForDelegate(_streamFailedCallback)
            };

            player.Initialize(playerParams);

            return player;
        }

        public static readonly RoutedEvent StreamStartedEvent =
            EventManager.RegisterRoutedEvent("StreamStarted", RoutingStrategy.Bubble,
            typeof(RoutedEventHandler), typeof(StreamPlayerControl));

        /// <summary>
        /// Occurs when the first frame is read from a stream.
        /// </summary>
        public event RoutedEventHandler StreamStarted
        {
            add { AddHandler(StreamStartedEvent, value); }
            remove { RemoveHandler(StreamStartedEvent, value); }
        }

        private void RaiseStreamStartedEvent()
        {
            IsPlaying = true;
            RaiseEvent(new RoutedEventArgs(StreamStartedEvent));
        }

        public static readonly RoutedEvent StreamStoppedEvent =
            EventManager.RegisterRoutedEvent("StreamStopped", RoutingStrategy.Bubble,
            typeof(RoutedEventHandler), typeof(StreamPlayerControl));

        /// <summary>
        /// Occurs when there are no more frames to read from a stream.
        /// </summary>
        public event RoutedEventHandler StreamStopped
        {
            add { AddHandler(StreamStoppedEvent, value); }
            remove { RemoveHandler(StreamStoppedEvent, value); }
        }

        private void RaiseStreamStoppedEvent()
        {
            IsPlaying = false;
            RaiseEvent(new RoutedEventArgs(StreamStoppedEvent));
        }

        public static readonly RoutedEvent StreamFailedEvent =
            EventManager.RegisterRoutedEvent("StreamFailed", RoutingStrategy.Bubble,
            typeof(RoutedEventHandler), typeof(StreamPlayerControl));

        /// <summary>
        /// Occurs when the player fails to play a stream.
        /// </summary>
        public event RoutedEventHandler StreamFailed
        {
            add { AddHandler(StreamFailedEvent, value); }
            remove { RemoveHandler(StreamFailedEvent, value); }
        }

        private void RaiseStreamFailedEvent()
        {
            IsPlaying = false;
            RaiseEvent(new RoutedEventArgs(StreamFailedEvent));
        }
    }
}
