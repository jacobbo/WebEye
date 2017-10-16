namespace WebEye.Controls.Wpf.StreamPlayerControl
{
    using System.Windows;

    public class StreamFailedEventArgs : RoutedEventArgs
    {
        private readonly string _error;

        /// <summary>
        /// Gets the error message.
        /// </summary>
        public string Error
        {
            get { return _error; }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="StreamFailedEventArgs"/> class.
        /// </summary>
        /// <param name="routedEvent">
        /// The routed event.
        /// </param>
        /// <param name="error">
        /// The error message.
        /// </param>
        public StreamFailedEventArgs(RoutedEvent routedEvent,
            string error) : base(routedEvent)
        {
            _error = error;
        }
    }
}
