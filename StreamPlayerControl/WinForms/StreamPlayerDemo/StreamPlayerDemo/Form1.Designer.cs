namespace WindowsFormsApplication1
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.streamPlayerControl1 = new WebEye.StreamPlayerControl();
            this._urlTextBox = new System.Windows.Forms.TextBox();
            this._playButton = new System.Windows.Forms.Button();
            this._stopButton = new System.Windows.Forms.Button();
            this._imageButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // streamPlayerControl1
            // 
            this.streamPlayerControl1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.streamPlayerControl1.Location = new System.Drawing.Point(0, 0);
            this.streamPlayerControl1.Name = "streamPlayerControl1";
            this.streamPlayerControl1.Size = new System.Drawing.Size(813, 450);
            this.streamPlayerControl1.TabIndex = 0;
            // 
            // _urlTextBox
            // 
            this._urlTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this._urlTextBox.Location = new System.Drawing.Point(5, 457);
            this._urlTextBox.Name = "_urlTextBox";
            this._urlTextBox.Size = new System.Drawing.Size(576, 26);
            this._urlTextBox.TabIndex = 1;
            this._urlTextBox.Text = "rtsp://184.72.239.149/vod/mp4:BigBuckBunny_115k.mov";
            // 
            // _playButton
            // 
            this._playButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this._playButton.Location = new System.Drawing.Point(587, 456);
            this._playButton.Name = "_playButton";
            this._playButton.Size = new System.Drawing.Size(75, 37);
            this._playButton.TabIndex = 2;
            this._playButton.Text = "Play";
            this._playButton.UseVisualStyleBackColor = true;
            this._playButton.Click += new System.EventHandler(this.playButton_Click);
            // 
            // _stopButton
            // 
            this._stopButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this._stopButton.Enabled = false;
            this._stopButton.Location = new System.Drawing.Point(662, 456);
            this._stopButton.Name = "_stopButton";
            this._stopButton.Size = new System.Drawing.Size(75, 37);
            this._stopButton.TabIndex = 3;
            this._stopButton.Text = "Stop";
            this._stopButton.UseVisualStyleBackColor = true;
            this._stopButton.Click += new System.EventHandler(this.stopButton_Click);
            // 
            // _imageButton
            // 
            this._imageButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this._imageButton.Enabled = false;
            this._imageButton.Location = new System.Drawing.Point(737, 456);
            this._imageButton.Name = "_imageButton";
            this._imageButton.Size = new System.Drawing.Size(75, 37);
            this._imageButton.TabIndex = 4;
            this._imageButton.Text = "Image...";
            this._imageButton.UseVisualStyleBackColor = true;
            this._imageButton.Click += new System.EventHandler(this.imageButton_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(814, 494);
            this.Controls.Add(this._imageButton);
            this.Controls.Add(this._stopButton);
            this.Controls.Add(this._playButton);
            this.Controls.Add(this._urlTextBox);
            this.Controls.Add(this.streamPlayerControl1);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private WebEye.StreamPlayerControl streamPlayerControl1;
        private System.Windows.Forms.TextBox _urlTextBox;
        private System.Windows.Forms.Button _playButton;
        private System.Windows.Forms.Button _stopButton;
        private System.Windows.Forms.Button _imageButton;
    }
}

