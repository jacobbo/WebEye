using System.Drawing;
using System.Windows.Forms;

namespace WebEye.StreamControl.WinForms
{
    using System;
    using System.Threading;

    /// <summary>
    /// The stream player control.
    /// </summary>
    public partial class StreamControl: UserControl
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="StreamControl"/> class.
        /// </summary>
        public StreamControl()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Gets or sets a value indicating whether to preserve the aspect ratio of a stream.
        /// </summary>
        public Boolean PreserveStreamAspectRatio { get; set; }

        private SynchronizationContext _uiContext = SynchronizationContext.Current;

        private Stream _stream;
        
        public void AttachStream(Stream stream)
        {
            if (_stream != null)
            {
                _stream.FrameRecieved -= HandleFrameRecieved;
            }

            _stream = stream;

            if (_stream != null)
            {
                _stream.FrameRecieved += HandleFrameRecieved;
            }
        }

        public void DetachStream()
        {
            if (_stream != null)
            {
                _stream.FrameRecieved -= HandleFrameRecieved;
            }
        }

        private Bitmap _currentFrame;

        private void HandleFrameRecieved(object sender, FrameRecievedEventArgs e)
        {
            _uiContext.Post(o =>
            {
                _currentFrame?.Dispose();
                _currentFrame = e.Frame;

                Invalidate();
            }, null);
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e); 

            if (_currentFrame != null)
            {
                if (PreserveStreamAspectRatio)
                {
                    float scale = Math.Min(e.ClipRectangle.Width / (float)_currentFrame.Width,
                        e.ClipRectangle.Height / (float)_currentFrame.Height);

                    var scaleWidth = (int)(_currentFrame.Width * scale);
                    var scaleHeight = (int)(_currentFrame.Height * scale);

                    e.Graphics.DrawImage(_currentFrame, ((int)e.ClipRectangle.Width - scaleWidth) / 2,
                        ((int)e.ClipRectangle.Height - scaleHeight) / 2, scaleWidth, scaleHeight);
                }
                else
                {
                    e.Graphics.DrawImage(_currentFrame, e.ClipRectangle);
                }
            }            
        }

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                _currentFrame?.Dispose();

                DetachStream();
            }

            if (disposing && (components != null))
            {
                components.Dispose();
            }

            base.Dispose(disposing);
        }        
    }
}
