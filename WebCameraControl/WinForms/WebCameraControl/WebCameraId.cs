namespace WebEye.Controls.WinForms.WebCameraControl
{
    using System;

    public sealed class WebCameraId : IEquatable<WebCameraId>
    {
        /// <summary>
        /// Initializes a new instance of the WebCameraId class.
        /// </summary>
        internal WebCameraId(DirectShowProxy.VideoInputDeviceInfo info)
        {
            this._name = info.FriendlyName;
            this._devicePath = info.DevicePath;
        }

        private readonly String _name;

        /// <summary>
        /// Gets the name of the web camera.
        /// </summary>
        public String Name
        {
            get { return this._name; }
        }

        private readonly String _devicePath;

        /// <summary>
        /// Gets the device path of the web camera.
        /// </summary>
        internal String DevicePath
        {
            get { return this._devicePath; }
        }

        public override Boolean Equals(object obj)
        {
            if (ReferenceEquals(null, obj)) return false;
            if (ReferenceEquals(this, obj)) return true;
            if (obj.GetType() != typeof(WebCameraId)) return false;
            return this.Equals((WebCameraId)obj);
        }

        /// <summary>
        /// Indicates whether the current object is equal to another object of the same type.
        /// </summary>
        /// <returns>
        /// true if the current object is equal to the <paramref name="other"/> parameter; otherwise, false.
        /// </returns>
        /// <param name="other">An object to compare with this object.</param>
        public bool Equals(WebCameraId other)
        {
            if (ReferenceEquals(null, other)) return false;
            if (ReferenceEquals(this, other)) return true;
            return Equals(other._name, this._name) && Equals(other._devicePath, this._devicePath);
        }

        /// <summary>
        /// Serves as a hash function for a particular type. 
        /// </summary>
        /// <returns>
        /// A hash code for the current <see cref="T:System.Object"/>.
        /// </returns>
        public override Int32 GetHashCode()
        {
            unchecked
            {
                return (this._name.GetHashCode() * 397) ^ this._devicePath.GetHashCode();
            }
        }

        public static Boolean operator ==(WebCameraId left, WebCameraId right)
        {
            return Equals(left, right);
        }

        public static Boolean operator !=(WebCameraId left, WebCameraId right)
        {
            return !Equals(left, right);
        }
    }
}
