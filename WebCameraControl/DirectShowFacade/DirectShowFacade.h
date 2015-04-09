/******************************************************************************
This module implements a set of utilites to work with a DirectShow capture graph.
The graph is used to output a video stream from a video capture device to a window.

A typical use-case scenario is the following:
1. BuildCaptureGraph()
2. AddRenderFilter(hWnd)
3. AddCaptureFilter(devicePath)
4. Start()
5. Stop()
6. DestroyCaptureGraph()

In case you want to change a video capture device you should:
1. call the ResetCaptureGraph() function
2. repeat steps 3-5

A list of available video capture devices can be retrieved using
the EnumVideoInputDevices function. 

Copyright (C) 2012 Alexander Iacobciuc (a.iacobciuc@gmail.com)
******************************************************************************/

#ifndef DIRECTSHOWFACADE_H
#define DIRECTSHOWFACADE_H

#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#include <WTypes.h>

struct VideoInputDeviceInfo;
typedef void (__stdcall *EnumVideoInputDevicesCallback)(const VideoInputDeviceInfo *pInfo);

/// <summary>
/// Enumerates video input devices in a system.
/// </summary>
/// <param name="callback">A callback method.</param>
void __stdcall EnumVideoInputDevices(EnumVideoInputDevicesCallback callback);

/// <summary>
/// Builds a video capture graph.
/// </summary>
/// <returns>If the function succeeds, the return value is zero.</returns>
int __stdcall BuildCaptureGraph();

/// <summary>
/// Adds a renderer filter to a video capture graph, which renders a video stream within a container window.
/// </summary>
/// <param name="hWnd">A container window that video should be clipped to.</param>
/// <returns>If the function succeeds, the return value is zero.</returns>
int __stdcall AddRenderFilter(HWND hWnd);

/// <summary>
/// Adds a video stream source to a video capture graph.
/// </summary>
/// <param name="devicePath">A device path of a video capture filter to add.</param>
/// <returns>If the function succeeds, the return value is zero.</returns>
int __stdcall AddCaptureFilter(BSTR devicePath);

/// <summary>
/// Removes a video stream source from a video capture graph.
/// </summary>
/// <returns>If the function succeeds, the return value is zero.</returns>
int __stdcall ResetCaptureGraph();

/// <summary>
/// Runs all the filters in a video capture graph. While the graph is running,
/// data moves through the graph and is rendered. 
/// </summary>
/// <returns>If the function succeeds, the return value is zero.</returns>
int __stdcall Start();

/// <summary>
/// Stops all the filters in a video capture graph.
/// </summary>
/// <returns>If the function succeeds, the return value is zero.</returns>
int __stdcall Stop();

/// <summary>
/// Retrieves the current image being displayed by the renderer filter.
/// </summary>
/// <param name="ppDib">Address of a pointer to a BYTE that will receive the DIB.</param>
/// <returns>If the function succeeds, the return value is zero.</returns>
int __stdcall GetCurrentImage(BYTE **ppDib);

/// <summary>
/// Retrieves the unstretched video size.
/// </summary>
/// <param name="lpWidth">A pointer to a LONG that will receive the width.</param>
/// <param name="lpHeight">A pointer to a LONG that will receive the height.</param>
/// <returns>If the function succeeds, the return value is zero.</returns>
int __stdcall GetVideoSize(LONG *lpWidth, LONG *lpHeight);

/// <summary>
/// Destroys a video capture graph.
/// </summary>
void __stdcall DestroyCaptureGraph();

#endif // DIRECTSHOWFACADE_H