using System.Collections.ObjectModel;
using WebEye;

namespace WebEye.StreamControl.Wpf.Demo
{
    public class LocalStreamViewModel : StreamViewModelBase
    {
        public ObservableCollection<Stream> LocalStreams { get; }
            = new ObservableCollection<Stream>(Stream.GetLocalStreams());        
    }
}
