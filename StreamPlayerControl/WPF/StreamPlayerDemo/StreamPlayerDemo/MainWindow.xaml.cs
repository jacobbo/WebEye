using System.Windows;
using Microsoft.Win32;

namespace StreamPlayerDemo
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void HandlePlayButtonClick(object sender, RoutedEventArgs e)
        {
            streamPlayerControl.StartPlay(_urlTextBox.Text);
        }

        private void HandleStopButtonClick(object sender, RoutedEventArgs e)
        {
            streamPlayerControl.Stop();

            UpdateButtons();
        }

        private void HandleImageButtonClick(object sender, RoutedEventArgs e)
        {
            var dialog = new SaveFileDialog { Filter = "Bitmap Image|*.bmp" };
            if (dialog.ShowDialog() == true)
            {
                streamPlayerControl.GetCurrentFrame().Save(dialog.FileName);
            }
        }

        private void UpdateButtons()
        {
            _playButton.IsEnabled = !streamPlayerControl.IsPlaying; 
            _stopButton.IsEnabled = streamPlayerControl.IsPlaying;
            _imageButton.IsEnabled = streamPlayerControl.IsPlaying;
        }

        private void HandlePlayFailed(object sender, RoutedEventArgs e)
        {
            //throw new System.NotImplementedException();
            UpdateButtons();
        }

        private void HandlePlaySucceeded(object sender, RoutedEventArgs e)
        {
            //throw new System.NotImplementedException();
            UpdateButtons();
        }
    }
}
