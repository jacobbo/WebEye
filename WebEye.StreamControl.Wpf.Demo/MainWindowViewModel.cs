using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Input;

namespace WebEye.StreamControl.Wpf.Demo
{
    public class MainWindowViewModel : NotifyPropertyChangedBase, IDisposable
    {
        public MainWindowViewModel()
        {
            AddLocalStreamCommand = 
                new RelayCommand(p => Streams.Add(new LocalStreamViewModel()));

            AddUriStreamCommand =
                new RelayCommand(p => Streams.Add(new UriStreamViewModel()));
        }

        public ObservableCollection<StreamViewModelBase> Streams { get; }
            = new ObservableCollection<StreamViewModelBase>();

        public ICommand AddLocalStreamCommand { get; }

        public ICommand AddUriStreamCommand { get; }

        public void Dispose()
        {
            foreach (var disposable in Streams.OfType<IDisposable>())
            {
                disposable.Dispose();
            }
        }
    }
}
