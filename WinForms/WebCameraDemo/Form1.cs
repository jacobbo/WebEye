using System;
using System.Windows.Forms;
using WebEye;

namespace WebCameraDemo
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        /// <summary>
        /// The helper class for a combo box item.
        /// </summary>
        private class ComboBoxItem
        {
            public ComboBoxItem(WebCameraId id)
            {
                _id = id;
            }

            private readonly WebCameraId _id;
            public WebCameraId Id
            {
                get { return _id; }
            }

            public override string ToString()
            {
                // Generates the text shown in the combo box.
                return _id.Name;
            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            foreach (WebCameraId camera in webCameraControl1.GetVideoCaptureDevices())
            {
                comboBox1.Items.Add(new ComboBoxItem (camera));
            }

            if (comboBox1.Items.Count > 0)
            {
                comboBox1.SelectedItem = comboBox1.Items[0];
            }
        }

        private void startButton_Click(object sender, EventArgs e)
        {
            ComboBoxItem i = (ComboBoxItem)comboBox1.SelectedItem;

            try
            {
                webCameraControl1.StartCapture(i.Id);
            }
            finally
            {
                UpdateButtons();
            }
        }

        private void stopButton_Click(object sender, EventArgs e)
        {
            webCameraControl1.StopCapture();

            UpdateButtons();
        }

        private void imageButton_Click(object sender, EventArgs e)
        {
            SaveFileDialog saveFileDialog1 = new SaveFileDialog();
            saveFileDialog1.Filter = "Bitmap Image|*.bmp";
            saveFileDialog1.Title = "Save an Image File";
            if (saveFileDialog1.ShowDialog() == DialogResult.OK)
            {
                webCameraControl1.GetCurrentImage().Save(saveFileDialog1.FileName);
            }
        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            UpdateButtons();
        }

        private void UpdateButtons()
        {
            startButton.Enabled = comboBox1.SelectedItem != null;
            stopButton.Enabled = webCameraControl1.IsCapturing;
            imageButton.Enabled = webCameraControl1.IsCapturing;
        }
    }
}
