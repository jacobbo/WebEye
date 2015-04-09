#include "DirectShowFacade.h"

#define _ATL_NO_AUTOMATIC_NAMESPACE
#include <atlbase.h>

#include <dshow.h>
#include <D3d9.h>
#include <Vmr9.h>

#pragma comment(lib, "strmiids")

using namespace ATL;

CComPtr<IGraphBuilder> g_spGraphBuilder;
CComPtr<ICaptureGraphBuilder2> g_spCaptureGraphBuilder;
CComPtr<IBaseFilter> g_spRenderFilter;
CComPtr<IVMRWindowlessControl9> g_spWindowlessControl;
CComPtr<IMediaControl> g_spMediaControl;
bool g_GraphIsRunning = false;

HWND g_hWnd = NULL;
WNDPROC g_OrigWndProc = NULL;

struct VideoInputDeviceInfo
{
    BSTR FriendlyName;
    BSTR Path;
};

void ReadDeviceInformation(IEnumMoniker *pEnum, EnumVideoInputDevicesCallback callback)
{
    ATLASSERT(pEnum != NULL);
    ATLASSERT(callback != NULL);

    CComPtr<IMoniker> spMoniker;
    while ((pEnum != NULL) && (pEnum->Next(1, &spMoniker, NULL) == S_OK))
    {
        CComPtr<IPropertyBag> spPropBag;
        HRESULT hr = spMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&spPropBag));
        if (FAILED(hr))
        {
            spMoniker.Release();
            continue;  
        } 

        CComBSTR friendlyName, path;
        CComVariant prop;

        // Get description or friendly name.
        hr = spPropBag->Read(L"FriendlyName", &prop, NULL);
        if (SUCCEEDED(hr) && (prop.vt == VT_BSTR))
        {
            hr = friendlyName.AssignBSTR(prop.bstrVal);
            ATLASSERT(SUCCEEDED(hr));
            hr = prop.Clear();
            ATLASSERT(SUCCEEDED(hr));
        }

        hr = spPropBag->Read(L"DevicePath", &prop, NULL);
        if (SUCCEEDED(hr) && (prop.vt == VT_BSTR))
        {
            // The device path is not intended for display.
            hr = path.AssignBSTR(prop.bstrVal);
            ATLASSERT(SUCCEEDED(hr));
        }

        if (callback != NULL)
        {
            const VideoInputDeviceInfo info = { friendlyName.Detach(), path.Detach() };
            callback(&info);
        }

        spMoniker.Release();
    }
}

HRESULT EnumDevices(REFGUID category, IEnumMoniker **ppEnum)
{
    ATLASSERT(ppEnum != NULL);

    // Create the System Device Enumerator.
    CComPtr<ICreateDevEnum> spDevEnum;
    HRESULT hr = spDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
    if (SUCCEEDED(hr))
    {
        // Create an enumerator for the category.
        hr = spDevEnum->CreateClassEnumerator(category, ppEnum, 0);
        if (hr == S_FALSE)
        {
            hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
        }
    }

    return hr;
}

void OnPaint(HWND hWnd) 
{ 
    if (g_spWindowlessControl != NULL && g_GraphIsRunning)
    {
        // Paint the window's client area. 
        PAINTSTRUCT ps; 
        HDC hdc = ::BeginPaint(hWnd, &ps); 

        // Request the VMR to paint the video.
        HRESULT hr = g_spWindowlessControl->RepaintVideo(hWnd, hdc);  
        ATLASSERT(SUCCEEDED(hr));
        UNREFERENCED_PARAMETER(hr); // Suppress a 'not referenced' warning in Release build. 

        ::EndPaint(hWnd, &ps); 
    }
} 

void OnSize(HWND hWnd) 
{ 
    if (g_spWindowlessControl != NULL)
    {
        // Set the size and position of the window.
        RECT rcDest = {0, 0, 0, 0};
        // Get the window client area.
        ::GetClientRect(hWnd, &rcDest); 
        // Set the video position.
        HRESULT hr = g_spWindowlessControl->SetVideoPosition(NULL, &rcDest); 
        ATLASSERT(SUCCEEDED(hr));
        UNREFERENCED_PARAMETER(hr); // Suppress a 'not referenced' warning in Release build.
    }
} 

void OnDisplayChange() 
{ 
    if (g_spWindowlessControl != NULL)
    {
        HRESULT hr = g_spWindowlessControl->DisplayModeChanged(); 
        ATLASSERT(SUCCEEDED(hr));
        UNREFERENCED_PARAMETER(hr); // Suppress a 'not referenced' warning in Release build.
    }
} 

LRESULT APIENTRY WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) 
    { 
    case WM_PAINT: 
        OnPaint(hWnd);
        break; 

    case WM_WINDOWPOSCHANGED: 
    case WM_SIZE: 
        OnSize(hWnd);
        break; 

    case WM_DISPLAYCHANGE: 
        OnDisplayChange();
        break; 
    case WM_ERASEBKGND:
        if (g_GraphIsRunning)
        {
            return 1;
        }
        break;

    default: break;            
    }  

    return CallWindowProc(g_OrigWndProc, hWnd, uMsg, wParam, lParam); 
}

void FindCaptureFilter(const CComBSTR &devicePath, IBaseFilter **ppvResult)
{
    CComPtr<IEnumMoniker> spEnum;
    HRESULT hr = ::EnumDevices(CLSID_VideoInputDeviceCategory, &spEnum);
    if (SUCCEEDED(hr) && spEnum != NULL)
    {
        CComPtr<IMoniker> spMoniker;
        while (spEnum->Next(1, &spMoniker, NULL) == S_OK)
        {
            CComPtr<IPropertyBag> spPropBag;
            hr = spMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&spPropBag));
            if (FAILED(hr))
            {
                spMoniker.Release();
                continue;  
            } 

            CComVariant prop;
            hr = spPropBag->Read(L"DevicePath", &prop, NULL);
            if (SUCCEEDED(hr) && (prop.vt == VT_BSTR))
            {
                CComBSTR path;
                hr = path.AssignBSTR(prop.bstrVal);
                ATLASSERT(SUCCEEDED(hr));

                if (path == devicePath)
                {
                    hr = spMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)ppvResult);
                    ATLASSERT(SUCCEEDED(hr));
                    break;
                }
            }

            spMoniker.Release();
        }
    }
}

void InvalidateWindow()
{
    if ((g_hWnd != NULL) && (g_OrigWndProc != NULL))
    {
        ::InvalidateRect(g_hWnd, NULL, TRUE);
    }
}

void __stdcall EnumVideoInputDevices(EnumVideoInputDevicesCallback callback)
{
    ATLASSERT(callback != NULL);

    CComPtr<IEnumMoniker> spEnum;

    HRESULT hr = ::EnumDevices(CLSID_VideoInputDeviceCategory, &spEnum);
    if (SUCCEEDED(hr) && (callback != NULL))
    {
        ::ReadDeviceInformation(spEnum, callback);
    }
}

int __stdcall BuildCaptureGraph()
{
    ::CoInitializeEx(NULL, COINIT_MULTITHREADED);

    // Create the Capture Graph Builder.
    HRESULT hr = g_spCaptureGraphBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
    if (SUCCEEDED(hr))
    {
        // Create the Filter Graph Manager.
        hr = g_spGraphBuilder.CoCreateInstance(CLSID_FilterGraph);
        if (SUCCEEDED(hr))
        {
            // Initialize the Capture Graph Builder.
            hr = g_spCaptureGraphBuilder->SetFiltergraph(g_spGraphBuilder);
            ATLASSERT(SUCCEEDED(hr));
        }
        else
        {
            g_spCaptureGraphBuilder.Release();
        }
    }

    return hr;
}

int __stdcall AddRenderFilter(HWND hWnd)
{
    HRESULT hr = g_spRenderFilter.CoCreateInstance(CLSID_VideoMixingRenderer9);
    if (SUCCEEDED(hr))
    {
        CComQIPtr<IVMRFilterConfig9> spConfig(g_spRenderFilter);
        if (spConfig != NULL && SUCCEEDED(hr))
        {
            hr = spConfig->SetRenderingMode(VMRMode_Windowless);
            ATLASSERT(SUCCEEDED(hr));

            g_spWindowlessControl = g_spRenderFilter;
            if (g_spWindowlessControl != NULL && SUCCEEDED(hr))
            {
                hr = g_spWindowlessControl->SetVideoClippingWindow(hWnd);
                ATLASSERT(SUCCEEDED(hr));

                RECT rcDest = {0, 0, 0, 0}; 
                // Get the window client area.
                ::GetClientRect(hWnd, &rcDest); 
                // Set the video position.
                hr = g_spWindowlessControl->SetVideoPosition(NULL, &rcDest); 
                ATLASSERT(SUCCEEDED(hr));

                g_hWnd = hWnd;
                g_OrigWndProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(hWnd, GWLP_WNDPROC,
                    reinterpret_cast<LONG_PTR>(WindowProc)));
            }
        }

        if (g_spGraphBuilder != NULL)
        {
            hr = g_spGraphBuilder->AddFilter(g_spRenderFilter, NULL);
            ATLASSERT(SUCCEEDED(hr));
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }

    return hr;
}

int __stdcall AddCaptureFilter(BSTR devicePath)
{
    CComBSTR path;
    HRESULT hr = path.AssignBSTR(devicePath);
    ATLASSERT(SUCCEEDED(hr));

    CComPtr<IBaseFilter> spCaptureFilter;
    ::FindCaptureFilter(path, &spCaptureFilter);
    if (spCaptureFilter != NULL)
    {
        hr = g_spGraphBuilder->AddFilter(spCaptureFilter, NULL);
        ATLASSERT(SUCCEEDED(hr));

        if (SUCCEEDED(hr) && (g_spCaptureGraphBuilder != NULL)
            && (spCaptureFilter != NULL) && (g_spRenderFilter != NULL))
        {
            hr = g_spCaptureGraphBuilder->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
                spCaptureFilter, NULL, g_spRenderFilter);

            if (FAILED(hr))
            {
                g_spGraphBuilder->RemoveFilter(spCaptureFilter);
            }
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }

    return hr;
}

int __stdcall Start()
{
    HRESULT hr(E_UNEXPECTED);
    if (g_spMediaControl == NULL)
    {
        g_spMediaControl = g_spGraphBuilder;
        ATLASSERT(g_spMediaControl != NULL);
    }

    if (g_spMediaControl != NULL)
    {
        hr = g_spMediaControl->Run();
        ATLASSERT(SUCCEEDED(hr));
        if (SUCCEEDED(hr))
        {
            g_GraphIsRunning = true;
        }
        else
        {
            InvalidateWindow();
        }
    }

    return hr;
}

int __stdcall Stop()
{
    HRESULT hr(E_UNEXPECTED);
    if (g_spMediaControl != NULL)
    {
        hr = g_spMediaControl->Stop();
        ATLASSERT(SUCCEEDED(hr));
        if (SUCCEEDED(hr))
        {
            g_GraphIsRunning = false;
        }

        InvalidateWindow();
    }

    return hr;
}

int __stdcall GetCurrentImage(BYTE **ppDib)
{
    HRESULT hr(E_UNEXPECTED);
    if (g_spWindowlessControl != NULL && g_GraphIsRunning)
    {
        hr = g_spWindowlessControl->GetCurrentImage(ppDib);
        ATLASSERT(SUCCEEDED(hr));
    }

    return hr;
}

int __stdcall GetVideoSize(LONG *lpWidth, LONG *lpHeight)
{
    HRESULT hr(E_UNEXPECTED);
    if (g_spWindowlessControl != NULL && g_GraphIsRunning)
    {
        LONG arWidth, arHeight;
        hr = g_spWindowlessControl->GetNativeVideoSize(lpWidth, lpHeight, &arWidth, &arHeight);
        ATLASSERT(SUCCEEDED(hr));
    }

    return hr;
}

int __stdcall ResetCaptureGraph()
{
    HRESULT hr(E_UNEXPECTED);
    if (g_spGraphBuilder != NULL)
    {
        // Remove all but the render filter.
        CComPtr<IEnumFilters> spEnum;
        hr = g_spGraphBuilder->EnumFilters(&spEnum);
        ATLASSERT(SUCCEEDED(hr));

        if (SUCCEEDED(hr))
        {
            CComPtr<IBaseFilter> spFilter;
            while (spEnum->Next(1, &spFilter, 0) == S_OK)
            {
                CComQIPtr<IVMRFilterConfig9> spRenderConfig(spFilter);
                if (spRenderConfig == NULL)
                {
                    // Remove the filter.
                    hr = g_spGraphBuilder->RemoveFilter(spFilter);
                    ATLASSERT(SUCCEEDED(hr));

                    // Reset the enumerator.
                    hr = spEnum->Reset();
                    ATLASSERT(SUCCEEDED(hr));
                }

                spFilter.Release();
            }
        }
    }

    return hr;
}

void __stdcall DestroyCaptureGraph()
{
    if ((g_hWnd != NULL) && (g_OrigWndProc != NULL))
    {
        ::SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_OrigWndProc));
        g_OrigWndProc = NULL;
        g_hWnd = NULL;
    }

    if (g_spWindowlessControl != NULL)
    {
        g_spWindowlessControl.Release();
        ATLASSERT(g_spWindowlessControl == NULL);
    }

    if (g_spMediaControl != NULL)
    {
        g_spMediaControl.Release();
        ATLASSERT(g_spMediaControl == NULL);
    }

    if (g_spRenderFilter != NULL)
    {
        g_spRenderFilter.Release();
        ATLASSERT(g_spRenderFilter == NULL);
    }

    if (g_spGraphBuilder != NULL)
    {
        g_spGraphBuilder.Release();
        ATLASSERT(g_spGraphBuilder == NULL);
    }

    if (g_spCaptureGraphBuilder != NULL)
    {
        g_spCaptureGraphBuilder.Release();
        ATLASSERT(g_spCaptureGraphBuilder == NULL);
    }

    ::CoUninitialize();
}