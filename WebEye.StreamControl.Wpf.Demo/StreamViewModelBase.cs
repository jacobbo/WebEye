using Microsoft.Win32;
using System;
using System.Drawing;
using System.Windows.Input;
using WebEye;

namespace WebEye.StreamControl.Wpf.Demo
{
    public abstract class StreamViewModelBase : NotifyPropertyChangedBase, IDisposable
    {
        public StreamViewModelBase()
        {
            StartStreamCommand = new RelayCommand(p => Stream.Start(),
                p => Stream != null);

            StopStreamCommand = new RelayCommand(p => Stream.Pause(),
                p => Stream != null);

            SaveFrameCommand = new RelayCommand(p => Stream.Resume(),
                p => Stream != null);

            //SaveFrameCommand = new RelayCommand(p =>
            //{
            //    var dialog = new SaveFileDialog { Filter = "Bitmap Image|*.bmp" };
            //    if (dialog.ShowDialog() == true)
            //    {
            //        _currentFrame.Save(dialog.FileName);
            //    }
            //}, p => _currentFrame != null);
        }

        private Stream _stream;
        public Stream Stream
        {
            get { return _stream; }
            set
            {
                UnSubscribeFromStreamEvents(_stream);
                SetField(ref _stream, value);
                SubscribeToStreamEvents(_stream);
            }
        }

        public ICommand StartStreamCommand { get; }

        public ICommand StopStreamCommand { get; }

        public ICommand SaveFrameCommand { get; }

        private void SubscribeToStreamEvents(Stream stream)
        {
            if (stream == null)
            {
                return;
            }

            stream.StreamFailed += HandleStreamFailed;
            stream.StreamStopped += HandleStreamStopped;
            stream.FrameRecieved += HandleFrameRecieved;
        }

        private void UnSubscribeFromStreamEvents(Stream stream)
        {
            if (stream == null)
            {
                return;
            }

            stream.StreamFailed -= HandleStreamFailed;
            stream.StreamStopped -= HandleStreamStopped;
            stream.FrameRecieved -= HandleFrameRecieved;
        }

        private Bitmap _currentFrame;

        private void HandleFrameRecieved(object sender, FrameRecievedEventArgs e)
        {
            Status = "Started";
            _currentFrame = e.Frame;
        }

        private void HandleStreamStopped(object sender, EventArgs e)
        {
            Status = "Stopped";
        }

        private void HandleStreamFailed(object sender, StreamFailedEventArgs e)
        {
            Status = $"Failed: {e.Error}";
        }

        public void Dispose()
        {
            Stream?.Dispose();
        }

        private String _status;
        public String Status
        {
            get { return _status; }
            set { SetField(ref _status, value); }
        }

        private Boolean _preserveAspectRatio;
        public Boolean PreserveAspectRatio
        {
            get { return _preserveAspectRatio; }
            set { SetField(ref _preserveAspectRatio, value); }
        }
    }
}
