using System;
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
            var uri = new Uri(_urlTextBox.Text);
            streamPlayerControl.StartPlay(uri);
        }

        private void HandleStopButtonClick(object sender, RoutedEventArgs e)
        {
            streamPlayerControl.Stop();
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

        private void HandlePlayerEvent(object sender, RoutedEventArgs e)
        {
            UpdateButtons();
        }
    }
}
