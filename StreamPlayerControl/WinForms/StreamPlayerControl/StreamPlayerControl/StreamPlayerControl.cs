using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace WebEye
{
    public partial class StreamPlayerControl: UserControl
    {
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
                    _player = new StreamPlayerProxy();
                    _player.Initialize(Handle);
                }

                return _player;
            }
        }

        /// <summary>
        /// Plays a stream.
        /// </summary>
        /// <param name="url">The url of a stream to play.</param>
        /// <exception cref="ArgumentException">An invalid string is passed as an argument.</exception>
        /// <exception cref="Win32Exception">Failed to load the FFmpeg facade dll.</exception>
        /// <exception cref="StreamPlayerException">Failed to play the stream.</exception>
        public void Play(String url)
        {
            if (IsPlaying)
            {
                Stop();
            }

            Player.Open(url);
            Player.Play();

            IsPlaying = true;
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

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
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

            base.Dispose(disposing);
        }
    }
}
