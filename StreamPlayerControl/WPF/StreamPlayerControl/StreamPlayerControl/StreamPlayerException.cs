using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WebEye.Controls.Wpf.StreamPlayerControl
{
    /// <summary>
    /// Represents the exception thrown when the stream player fails.
    /// </summary>
    public sealed class StreamPlayerException : Exception
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="StreamPlayerException"/> class.
        /// </summary>
        /// <param name="message">
        /// The message.
        /// </param>
        internal StreamPlayerException(string message)
            : base(message)
        {
        }
    }
}