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
            try
            {
                streamPlayerControl1.Play(_urlTextBox.Text);
            }
            finally
            {
                UpdateButtons();
            }
        }

        private void stopButton_Click(object sender, EventArgs e)
        {
            streamPlayerControl1.Stop();

            UpdateButtons();
        }

        private void imageButton_Click(object sender, EventArgs e)
        {
            SaveFileDialog saveFileDialog1 = new SaveFileDialog();
            saveFileDialog1.Filter = "Bitmap Image|*.bmp";
            saveFileDialog1.Title = "Save an Image File";
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
    }
}
