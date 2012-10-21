using System;
using System.Windows.Forms;

namespace WebCameraDemo
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.ThreadException += OnApplicationThreadException;

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
        }

        static void OnApplicationThreadException(object sender, System.Threading.ThreadExceptionEventArgs e)
        {
            string message = e.Exception.Message;
            if (e.Exception.InnerException != null)
            {
                message += Environment.NewLine + string.Format("[{0}]", e.Exception.InnerException.Message);
            }

            MessageBox.Show(message, "Exception!", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
    }
}
