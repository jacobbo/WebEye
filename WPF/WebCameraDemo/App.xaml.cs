using System;
using System.Windows;
using System.Windows.Threading;

namespace WpfWebCameraDemo
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App
    {
        void OnDispatcherUnhandledException(object sender, DispatcherUnhandledExceptionEventArgs e)
        {
            string message = e.Exception.Message;
            if (e.Exception.InnerException != null)
            {
                message += Environment.NewLine + string.Format("[{0}]", e.Exception.InnerException.Message);
            }

            MessageBox.Show(message, "Exception!", MessageBoxButton.OK, MessageBoxImage.Error);
            e.Handled = true;
        }
    }
}
