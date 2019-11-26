using System.Drawing;
using System.Windows.Forms;

namespace WebEye.StreamControl.WinForms
{
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
            }, null);
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e); 

            if (_currentFrame != null)
            {
                e.Graphics.DrawImage(_currentFrame, 0, 0);
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
