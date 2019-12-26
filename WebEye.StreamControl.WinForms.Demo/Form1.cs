using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;
using WebEye;

namespace WindowsFormsApplication1
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();

            var bindingSource1 = new BindingSource();
            bindingSource1.DataSource = _localStreams;

            comboBox1.DataSource = bindingSource1.DataSource;

            comboBox1.DisplayMember = "Name";
            comboBox1.ValueMember = "Name";
            _playButton2.Enabled = comboBox1.SelectedIndex >= 0;
        }

        private List<Stream> _localStreams = new List<Stream>(Stream.GetLocalStreams());

        private Stream _stream;
        public Stream Stream
        {
            get { return _stream; }
            set
            {
                UnSubscribeFromStreamEvents(_stream);
                 _stream = value;
                SubscribeToStreamEvents(_stream);
            }
        }

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
            BeginInvoke(new MethodInvoker(delegate ()
            {
                _currentFrame = e.Frame;

                if (_statusTextBox.Text != "Playing")
                {
                    _statusTextBox.Text = "Playing";
                    UpdateButtons();
                }
            }));
        }

        private void HandleStreamStopped(object sender, EventArgs e)
        {
            BeginInvoke(new MethodInvoker(delegate ()
            {
                _statusTextBox.Text = "Stopped";
                UpdateButtons();
            }));

        }

        private void HandleStreamFailed(object sender, StreamFailedEventArgs e)
        {
            BeginInvoke(new MethodInvoker(delegate ()
            {
                _statusTextBox.Text = $"Failed: {e.Error}";
                UpdateButtons();
            }));
        }

        private void playButton_Click(object sender, EventArgs e)
        {
            var uri = new Uri(_urlTextBox.Text);
            Stream = Stream.FromUri(uri);
            streamControl1.AttachStream(Stream);
            Stream.Start();
            _statusTextBox.Text = "Connecting...";
        }

        private void stopButton_Click(object sender, EventArgs e)
        {
            Stream?.Stop();
        }

        private void imageButton_Click(object sender, EventArgs e)
        {
            SaveFileDialog saveFileDialog1 = new SaveFileDialog();
            saveFileDialog1.Filter = "Bitmap Image|*.bmp";
            if (saveFileDialog1.ShowDialog() == DialogResult.OK)
            {
                _currentFrame?.Save(saveFileDialog1.FileName);
            }
        }

        private void UpdateButtons()
        {
            _playButton.Enabled = !string.IsNullOrEmpty(_urlTextBox.Text);
            _stopButton.Enabled = Stream != null;
            _imageButton.Enabled = _currentFrame != null;
        }

        private Bitmap _currentFrame2;

        private void playButton2_Click(object sender, EventArgs e)
        {
            if (comboBox1.SelectedIndex < 0)
                return;

            var stream = _localStreams[comboBox1.SelectedIndex];

            stream.FrameRecieved += (s, a) =>
            {
                BeginInvoke(new MethodInvoker(delegate ()
                {
                    _currentFrame2 = a.Frame;

                    if (_statusTextBox2.Text != "Playing")
                    {
                        _statusTextBox2.Text = "Playing";
                        UpdateButtons2();
                    }
                }));
            };

            stream.StreamStopped += (s, a) =>
            {
                BeginInvoke(new MethodInvoker(delegate ()
                {
                    _statusTextBox2.Text = "Stopped";
                    UpdateButtons2();
                }));
            };

            stream.StreamFailed += (s, a) =>
            {
                BeginInvoke(new MethodInvoker(delegate ()
                {
                    _statusTextBox2.Text = $"Failed: {a.Error}";
                    UpdateButtons2();
                }));
            };

            streamControl2.AttachStream(stream);
            stream.Start();
            _statusTextBox2.Text = "Connecting...";
        }

        private void stopButton2_Click(object sender, EventArgs e)
        {
            var stream = sender as Stream;
            stream?.Stop();
        }


        private void UpdateButtons2()
        {
            _playButton2.Enabled = comboBox1.SelectedIndex >= 0;
            _stopButton2.Enabled = _statusTextBox2.Text == "Playing";            
            _imageButton2.Enabled = _currentFrame2 != null;
        }


        private void Form1_FormClosed(object sender, System.Windows.Forms.FormClosedEventArgs e)
        {
            streamControl1.DetachStream();
            Stream?.Dispose();

            streamControl2.DetachStream();
            foreach (var s in _localStreams)
            {
                s.Dispose();
            }
        }

        private void HandleCheckedChanged(object sender, EventArgs e)
        {
            streamControl1.PreserveStreamAspectRatio = checkBox1.Checked;
        }

        private void checkBox2_CheckedChanged(object sender, EventArgs e)
        {
            streamControl2.PreserveStreamAspectRatio = checkBox2.Checked;
        }

        private void imageButton2_Click(object sender, EventArgs e)
        {
            SaveFileDialog saveFileDialog1 = new SaveFileDialog();
            saveFileDialog1.Filter = "Bitmap Image|*.bmp";
            if (saveFileDialog1.ShowDialog() == DialogResult.OK)
            {
                _currentFrame2?.Save(saveFileDialog1.FileName);
            }
        }

        //private void HandleStreamStartedEvent(object sender, EventArgs e)
        //{
        //    UpdateButtons();

        //    _statusTextBox.Text = "Playing";
        //}

        //private void HandleStreamFailedEvent(object sender, StreamFailedEventArgs e)
        //{
        //    UpdateButtons();

        //    _statusTextBox.Text = "Failed";

        //    MessageBox.Show(e.Error, "Stream Player Demo",
        //        MessageBoxButtons.OK, MessageBoxIcon.Error);
        //}

        //private void HandleStreamStoppedEvent(object sender, EventArgs e)
        //{
        //    UpdateButtons();

        //    _statusTextBox.Text = "Stopped";
        //}
    }
}
