using System;
using System.Windows.Forms;

namespace WindowsFormsApplication1
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void playButton_Click(object sender, EventArgs e)
        {
            var uri = new Uri(_urlTextBox.Text);
            streamPlayerControl1.StartPlay(uri);
        }

        private void stopButton_Click(object sender, EventArgs e)
        {
            streamPlayerControl1.Stop();
        }

        private void imageButton_Click(object sender, EventArgs e)
        {
            SaveFileDialog saveFileDialog1 = new SaveFileDialog();
            saveFileDialog1.Filter = "Bitmap Image|*.bmp";
            if (saveFileDialog1.ShowDialog() == DialogResult.OK)
            {
                streamPlayerControl1.GetCurrentFrame().Save(saveFileDialog1.FileName);
            }
        }

        private void UpdateButtons()
        {
            _playButton.Enabled = !streamPlayerControl1.IsPlaying;
            _stopButton.Enabled = streamPlayerControl1.IsPlaying;
            _imageButton.Enabled = streamPlayerControl1.IsPlaying;
        }

        private void HandlePlayerEvent(object sender, EventArgs e)
        {
            UpdateButtons();
        }
    }
}
