using System;
using System.Drawing;
using System.Threading;
using System.Windows;
using System.Windows.Media.Imaging;

namespace WebEye.StreamControl.Wpf
{
    /// <summary>
    /// Interaction logic for StreamControl.xaml
    /// </summary>
    public partial class StreamControl
    {
        private SynchronizationContext _uiContext = SynchronizationContext.Current;

        public StreamControl()
        {
            InitializeComponent();
        }
                
        private static void HandleStreamChanged(DependencyObject d,
            DependencyPropertyChangedEventArgs e)
        {
            var control = d as StreamControl;
            Stream oldStream = e.OldValue as Stream;
            if (oldStream != null && control != null)
            {
                oldStream.FrameRecieved -= control.HandleFrameRecieved;
            }

            Stream newStream = e.NewValue as Stream;
            if (newStream != null && control != null)
            {
                newStream.FrameRecieved += control.HandleFrameRecieved;
            }
        }

        private Bitmap _currentFrame;
        private WriteableBitmap _writeableBitmap;

        private void HandleFrameRecieved(object sender, FrameRecievedEventArgs e)
        {
            _uiContext.Post(o =>
            {
                _currentFrame?.Dispose();
                _currentFrame = e.Frame;

                if (_writeableBitmap == null)
                {
                    _writeableBitmap = _currentFrame.ToWriteableBitmap();
                    _image.Source = _writeableBitmap;
                }
                else
                {
                    _writeableBitmap.UpdateWith(_currentFrame);
                }
            }, null);
        }

        public static readonly DependencyProperty StreamProperty =
            DependencyProperty.Register("Stream", typeof(Stream), typeof(StreamControl),
        new FrameworkPropertyMetadata(null, HandleStreamChanged));

        /// <summary>
        /// Gets or sets a stream.
        /// </summary>
        public Stream Stream
        {
            get { return (Stream)GetValue(StreamProperty); }
            set { SetValue(StreamProperty, value); }
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
