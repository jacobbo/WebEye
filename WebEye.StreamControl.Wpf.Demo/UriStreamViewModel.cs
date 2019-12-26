using System;
using System.Windows.Input;

namespace WebEye.StreamControl.Wpf.Demo
{
    public class UriStreamViewModel : StreamViewModelBase
    {
        public UriStreamViewModel()
        {
            StartStreamCommand = new RelayCommand(p => {
                Stream?.Dispose();
                Stream = WebEye.Stream.FromUri(new Uri(Uri), TimeSpan.FromSeconds(10),
                    TimeSpan.FromSeconds(10), WebEye.RtspTransport.Undefined, WebEye.RtspFlags.None);
                Stream.Start();
            });
        }

        private String _uri = @"rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov";
        public String Uri
        {
            get { return _uri; }
            set { SetField(ref _uri, value); }
        }

        public new ICommand StartStreamCommand { get; }
    }
}
