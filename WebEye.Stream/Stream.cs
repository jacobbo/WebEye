using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Reflection;

namespace WebEye
{
    public class Stream : IDisposable
    {
        private Object _proxy;

        private Stream(Object proxy)
        {
            _proxy = proxy;

            EventInfo eventInfo = _proxy.GetType().GetEvent("FrameRecieved");
            MethodInfo method = typeof(Stream).GetMethod("RaiseFrameRecievedEvent",
                BindingFlags.NonPublic | BindingFlags.Instance);

            Delegate handler = Delegate.CreateDelegate(eventInfo.EventHandlerType, this, method);
            eventInfo.AddEventHandler(_proxy, handler);

            eventInfo = _proxy.GetType().GetEvent("StreamFailed");
            method = typeof(Stream).GetMethod("RaiseStreamFailedEvent",
    BindingFlags.NonPublic | BindingFlags.Instance);

            handler = Delegate.CreateDelegate(eventInfo.EventHandlerType, this, method);
            eventInfo.AddEventHandler(_proxy, handler);

            eventInfo = _proxy.GetType().GetEvent("StreamStopped");
            method = typeof(Stream).GetMethod("RaiseStreamStoppedEvent",
    BindingFlags.NonPublic | BindingFlags.Instance);

            handler = Delegate.CreateDelegate(eventInfo.EventHandlerType, this, method);
            eventInfo.AddEventHandler(_proxy, handler);
        }

        private void RaiseFrameRecievedEvent(Bitmap frame)
        {
            FrameRecieved?.Invoke(this, new FrameRecievedEventArgs(frame));
        }

        private void RaiseStreamFailedEvent(string error)
        {
            StreamFailed?.Invoke(this, new StreamFailedEventArgs(error));
        }

        private void RaiseStreamStoppedEvent()
        {
            StreamStopped?.Invoke(this, EventArgs.Empty);
        }

        /// <summary>
        /// Constructs a stream.
        /// </summary>
        /// <param name="uri">The uri of a stream to play.</param>
        /// <param name="connectionTimeout">The connection timeout.</param>
        /// <param name="streamTimeout">The stream timeout.</param>
        /// <param name="transport">RTSP transport protocol.</param>
        /// <param name="flags">RTSP flags.</param>     
        public static Stream FromUri(Uri uri,
            TimeSpan connectionTimeout, TimeSpan streamTimeout, RtspTransport transport, RtspFlags flags)
        {
            var type = Load();
            var proxy = type.GetMethod("FromUri").Invoke(null,
                new Object[] { uri, connectionTimeout, streamTimeout, transport, flags });

            return new Stream(proxy);
        }

        /// <summary>
        /// Constructs a stream.
        /// </summary>
        /// <param name="uri">The uri of a stream to play.</param>
        public static Stream FromUri(Uri uri)
        {
            return FromUri(uri, TimeSpan.FromSeconds(15),
                TimeSpan.FromSeconds(15), RtspTransport.Undefined, RtspFlags.None);
        }

        private static Type Load()
        {
            var path = Path.Combine(AppDomain.CurrentDomain.BaseDirectory,
                Environment.Is64BitProcess ? "WebEye.Stream.x64.dll" : "WebEye.Stream.Win32.dll");
            var name = AssemblyName.GetAssemblyName(path);
            return Assembly.Load(name).GetType("WebEye.ManagedWrapper");            
        }       

        /// <summary>
        /// Retrieves local streams, each represents a video input device connected to the system.
        /// </summary>
        public static IEnumerable<Stream> GetLocalStreams()
        {
            var type = Load();
            IEnumerable<Object> proxies = (IEnumerable<Object>)type.GetMethod("GetLocalStreams")
                .Invoke(null, null);

            foreach (var p in proxies)
            {
                yield return new Stream(p);
            }
        }

        public void Dispose()
        {
            var disposable = _proxy as IDisposable;
            disposable?.Dispose();
        }

        public string Name
        {
            get
            {
                return (string)_proxy.GetType().GetProperty("Name").GetValue(_proxy, null);
            }
        }

        /// <summary>
        /// Asynchronously plays a stream.
        /// </summary>
        public void Start()
        {
            _proxy.GetType().GetMethod("Start").Invoke(_proxy, null);
        }

        /// <summary>
        /// Stops a stream.
        /// </summary>
        public void Stop()
        {
            _proxy.GetType().GetMethod("Stop").Invoke(_proxy, null);
        }

        /// <summary>
        /// Occurs when a frame is read from a stream.
        /// </summary>
        public event EventHandler<FrameRecievedEventArgs> FrameRecieved;

        /// <summary>
        /// Occurs when the player fails to play a stream.
        /// </summary>
        public event EventHandler<StreamFailedEventArgs> StreamFailed;

        /// <summary>
        /// Occurs when there are no more frames to read from a stream.
        /// </summary>
        public event EventHandler StreamStopped;
    }
}
