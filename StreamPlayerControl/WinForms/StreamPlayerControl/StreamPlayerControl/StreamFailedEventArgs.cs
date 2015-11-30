namespace WebEye
{
    using System;

    /// <summary>
    /// The stream failed event args.
    /// </summary>
    public class StreamFailedEventArgs : EventArgs
    {
        /// <summary>
        /// The _error.
        /// </summary>
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
        /// <param name="error">
        /// The error message.
        /// </param>
        public StreamFailedEventArgs(string error)
        {
            _error = error;
        }
    }
}
