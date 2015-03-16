using System;
using System.Runtime.InteropServices;
using System.Windows.Interop;

namespace WebEye
{
    internal class VideoWindow : HwndHost
    {
        #region WinAPI Interop

        /// <summary>
        /// Window Styles. 
        /// </summary>
        [Flags]
        private enum WindowStyles : uint
        {
            /// <summary>The window is a child window.</summary>
            WS_CHILD = 0x40000000,

            /// <summary>The window is initially visible.</summary>
            WS_VISIBLE = 0x10000000,
        }

        IntPtr _hWnd = IntPtr.Zero;

        internal new IntPtr Handle { get { return _hWnd; } }

        /// <summary>
        /// The CreateWindowEx function creates an overlapped, pop-up, or child window with an extended window style; otherwise, this function is identical to the CreateWindow function. 
        /// </summary>
        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern IntPtr CreateWindowEx(
           UInt32 dwExStyle,
           String lpClassName,
           String lpWindowName,
           WindowStyles dwStyle,
           Int32 x,
           Int32 y,
           Int32 nWidth,
           Int32 nHeight,
           IntPtr hWndParent,
           IntPtr hMenu,
           IntPtr hInstance,
           IntPtr lpParam);

        [StructLayout(LayoutKind.Sequential)]
        private struct RECT
        {
            internal Int32 left, top, right, bottom;
        }

        [DllImport("user32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern Boolean GetClientRect(IntPtr hWnd, out RECT lpRect);

        [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern Boolean DestroyWindow(IntPtr hWnd);

        #endregion

        #region Overrides of HwndHost

        /// <summary>
        /// Creates the window to be hosted. 
        /// </summary>
        /// <returns>
        /// The handle to the child Win32 window to create.
        /// </returns>
        /// <param name="hWndParent">The window handle of the parent window.</param>
        protected override HandleRef BuildWindowCore(HandleRef hWndParent)
        {
            RECT clientArea;
            GetClientRect(hWndParent.Handle, out clientArea);

            _hWnd = CreateWindowEx(0, "Static", "", WindowStyles.WS_CHILD | WindowStyles.WS_VISIBLE,
                            0, 0, clientArea.right - clientArea.left, clientArea.bottom - clientArea.top,
                            hWndParent.Handle, IntPtr.Zero, IntPtr.Zero, IntPtr.Zero);

            return new HandleRef(this, _hWnd);
        }

        /// <summary>
        /// Destroys the hosted window. 
        /// </summary>
        /// <param name="hWnd">A structure that contains the window handle.</param>
        protected override void DestroyWindowCore(HandleRef hWnd)
        {
            DestroyWindow(hWnd.Handle);
        }

        #endregion
    }
}
