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
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.streamControl1 = new WebEye.StreamControl.WinForms.StreamControl();
            this.checkBox1 = new System.Windows.Forms.CheckBox();
            this._imageButton = new System.Windows.Forms.Button();
            this._statusTextBox = new System.Windows.Forms.TextBox();
            this._urlTextBox = new System.Windows.Forms.TextBox();
            this._playButton = new System.Windows.Forms.Button();
            this._stopButton = new System.Windows.Forms.Button();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.comboBox1 = new System.Windows.Forms.ComboBox();
            this.checkBox2 = new System.Windows.Forms.CheckBox();
            this._imageButton2 = new System.Windows.Forms.Button();
            this._statusTextBox2 = new System.Windows.Forms.TextBox();
            this._playButton2 = new System.Windows.Forms.Button();
            this._stopButton2 = new System.Windows.Forms.Button();
            this.streamControl2 = new WebEye.StreamControl.WinForms.StreamControl();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabControl1
            // 
            this.tabControl1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Location = new System.Drawing.Point(6, 12);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(800, 413);
            this.tabControl1.TabIndex = 8;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.streamControl1);
            this.tabPage1.Controls.Add(this.checkBox1);
            this.tabPage1.Controls.Add(this._imageButton);
            this.tabPage1.Controls.Add(this._statusTextBox);
            this.tabPage1.Controls.Add(this._urlTextBox);
            this.tabPage1.Controls.Add(this._playButton);
            this.tabPage1.Controls.Add(this._stopButton);
            this.tabPage1.Location = new System.Drawing.Point(4, 29);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(792, 380);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Remote Stream";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // streamControl1
            // 
            this.streamControl1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.streamControl1.Location = new System.Drawing.Point(6, 6);
            this.streamControl1.Name = "streamControl1";
            this.streamControl1.PreserveStreamAspectRatio = false;
            this.streamControl1.Size = new System.Drawing.Size(780, 322);
            this.streamControl1.TabIndex = 6;
            // 
            // checkBox1
            // 
            this.checkBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.checkBox1.AutoSize = true;
            this.checkBox1.Location = new System.Drawing.Point(327, 345);
            this.checkBox1.Name = "checkBox1";
            this.checkBox1.Size = new System.Drawing.Size(127, 24);
            this.checkBox1.TabIndex = 7;
            this.checkBox1.Text = "Aspect Ratio";
            this.checkBox1.UseVisualStyleBackColor = true;
            this.checkBox1.CheckedChanged += new System.EventHandler(this.HandleCheckedChanged);
            // 
            // _imageButton
            // 
            this._imageButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this._imageButton.Enabled = false;
            this._imageButton.Location = new System.Drawing.Point(721, 340);
            this._imageButton.Name = "_imageButton";
            this._imageButton.Size = new System.Drawing.Size(75, 37);
            this._imageButton.TabIndex = 4;
            this._imageButton.Text = "Image...";
            this._imageButton.UseVisualStyleBackColor = true;
            this._imageButton.Click += new System.EventHandler(this.imageButton_Click);
            // 
            // _statusTextBox
            // 
            this._statusTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this._statusTextBox.Enabled = false;
            this._statusTextBox.Location = new System.Drawing.Point(461, 341);
            this._statusTextBox.Name = "_statusTextBox";
            this._statusTextBox.Size = new System.Drawing.Size(104, 26);
            this._statusTextBox.TabIndex = 5;
            // 
            // _urlTextBox
            // 
            this._urlTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this._urlTextBox.Location = new System.Drawing.Point(3, 343);
            this._urlTextBox.Name = "_urlTextBox";
            this._urlTextBox.Size = new System.Drawing.Size(315, 26);
            this._urlTextBox.TabIndex = 1;
            this._urlTextBox.Text = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov";
            // 
            // _playButton
            // 
            this._playButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this._playButton.Location = new System.Drawing.Point(571, 340);
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
            this._stopButton.Location = new System.Drawing.Point(646, 340);
            this._stopButton.Name = "_stopButton";
            this._stopButton.Size = new System.Drawing.Size(75, 37);
            this._stopButton.TabIndex = 3;
            this._stopButton.Text = "Stop";
            this._stopButton.UseVisualStyleBackColor = true;
            this._stopButton.Click += new System.EventHandler(this.stopButton_Click);
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.comboBox1);
            this.tabPage2.Controls.Add(this.checkBox2);
            this.tabPage2.Controls.Add(this._imageButton2);
            this.tabPage2.Controls.Add(this._statusTextBox2);
            this.tabPage2.Controls.Add(this._playButton2);
            this.tabPage2.Controls.Add(this._stopButton2);
            this.tabPage2.Controls.Add(this.streamControl2);
            this.tabPage2.Location = new System.Drawing.Point(4, 29);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(792, 380);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Local Stream";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // comboBox1
            // 
            this.comboBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.comboBox1.FormattingEnabled = true;
            this.comboBox1.Location = new System.Drawing.Point(7, 340);
            this.comboBox1.Name = "comboBox1";
            this.comboBox1.Size = new System.Drawing.Size(302, 28);
            this.comboBox1.TabIndex = 13;
            // 
            // checkBox2
            // 
            this.checkBox2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.checkBox2.AutoSize = true;
            this.checkBox2.Location = new System.Drawing.Point(315, 342);
            this.checkBox2.Name = "checkBox2";
            this.checkBox2.Size = new System.Drawing.Size(127, 24);
            this.checkBox2.TabIndex = 12;
            this.checkBox2.Text = "Aspect Ratio";
            this.checkBox2.UseVisualStyleBackColor = true;
            this.checkBox2.CheckedChanged += new System.EventHandler(this.checkBox2_CheckedChanged);
            // 
            // _imageButton2
            // 
            this._imageButton2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this._imageButton2.Enabled = false;
            this._imageButton2.Location = new System.Drawing.Point(709, 337);
            this._imageButton2.Name = "_imageButton2";
            this._imageButton2.Size = new System.Drawing.Size(75, 37);
            this._imageButton2.TabIndex = 10;
            this._imageButton2.Text = "Image...";
            this._imageButton2.UseVisualStyleBackColor = true;
            this._imageButton2.Click += new System.EventHandler(this.imageButton2_Click);
            // 
            // _statusTextBox2
            // 
            this._statusTextBox2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this._statusTextBox2.Enabled = false;
            this._statusTextBox2.Location = new System.Drawing.Point(449, 338);
            this._statusTextBox2.Name = "_statusTextBox2";
            this._statusTextBox2.Size = new System.Drawing.Size(104, 26);
            this._statusTextBox2.TabIndex = 11;
            // 
            // _playButton2
            // 
            this._playButton2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this._playButton2.Location = new System.Drawing.Point(559, 337);
            this._playButton2.Name = "_playButton2";
            this._playButton2.Size = new System.Drawing.Size(75, 37);
            this._playButton2.TabIndex = 8;
            this._playButton2.Text = "Play";
            this._playButton2.UseVisualStyleBackColor = true;
            this._playButton2.Click += new System.EventHandler(this.playButton2_Click);
            // 
            // _stopButton2
            // 
            this._stopButton2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this._stopButton2.Enabled = false;
            this._stopButton2.Location = new System.Drawing.Point(634, 337);
            this._stopButton2.Name = "_stopButton2";
            this._stopButton2.Size = new System.Drawing.Size(75, 37);
            this._stopButton2.TabIndex = 9;
            this._stopButton2.Text = "Stop";
            this._stopButton2.UseVisualStyleBackColor = true;
            this._stopButton2.Click += new System.EventHandler(this.stopButton2_Click);
            // 
            // streamControl2
            // 
            this.streamControl2.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.streamControl2.Location = new System.Drawing.Point(6, 6);
            this.streamControl2.Name = "streamControl2";
            this.streamControl2.PreserveStreamAspectRatio = false;
            this.streamControl2.Size = new System.Drawing.Size(780, 325);
            this.streamControl2.TabIndex = 7;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(814, 430);
            this.Controls.Add(this.tabControl1);
            this.Name = "Form1";
            this.Text = "Form1";
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage1.PerformLayout();
            this.tabPage2.ResumeLayout(false);
            this.tabPage2.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.ComboBox comboBox1;
        private System.Windows.Forms.CheckBox checkBox2;
        private System.Windows.Forms.Button _imageButton2;
        private System.Windows.Forms.TextBox _statusTextBox2;
        private System.Windows.Forms.Button _playButton2;
        private System.Windows.Forms.Button _stopButton2;
        private WebEye.StreamControl.WinForms.StreamControl streamControl2;
        private System.Windows.Forms.TabPage tabPage1;
        private WebEye.StreamControl.WinForms.StreamControl streamControl1;
        private System.Windows.Forms.CheckBox checkBox1;
        private System.Windows.Forms.Button _imageButton;
        private System.Windows.Forms.TextBox _statusTextBox;
        private System.Windows.Forms.TextBox _urlTextBox;
        private System.Windows.Forms.Button _playButton;
        private System.Windows.Forms.Button _stopButton;
    }
}

