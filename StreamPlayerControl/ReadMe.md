The StreamPlayerControl is a FFmpeg-based stream player control, a simple and easy to use one, no additional dependencies and a minimalistic interface.

The repository has the following structure.

1. StreamPlayer is a FFmpeg façade, provides a simplified interface to a stream playback functionality.

2. WPF
  1. StreamPlayerControl is a WPF version of the control.
  2. StreamPlayerDemo is a very basic WPF application that shows how to use the control.
    
3. WinForms
  1. StreamPlayerControl is a WinForms version of the control.
  2. StreamPlayerDemo is a very basic WinForms application that shows how to use the control.    

More details about the control and its implementation can be found on the following page:
http://www.codeproject.com/Articles/885869/Stream-Player-control

The FFmpeg façade sources, the same as the FFmpeg framework, are licensed under [The LGPL license] (http://www.gnu.org/licenses/lgpl-3.0.html). 
The .NET controls' sources and demos' sources are licensed under [The Code Project Open License] (http://www.codeproject.com/info/cpol10.aspx) (CPOL).
The main points subject to the terms of the CPOL are:

* Source Code and Executable Files can be used in commercial applications;
* Source Code and Executable Files can be redistributed; and
* Source Code can be modified to create derivative works.
* No claim of suitability, guarantee, or any warranty whatsoever is provided. The software is provided "as-is".