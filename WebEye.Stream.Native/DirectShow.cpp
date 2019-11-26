#include "DirectShow.h"

#include <dshow.h>

using namespace std;
using namespace ATL;
using namespace WebEye;

void DirectShow::ReadDeviceInformation(IEnumMoniker *pEnum,
	vector<wstring> &devices)
{
	ATLASSERT(pEnum != NULL);

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

		CComBSTR friendlyName;
		CComVariant prop;

		// Get description or friendly name.
		hr = spPropBag->Read(L"FriendlyName", &prop, NULL);
		if (SUCCEEDED(hr) && (prop.vt == VT_BSTR))
		{
			hr = friendlyName.AssignBSTR(prop.bstrVal);
			ATLASSERT(SUCCEEDED(hr));
			hr = prop.Clear();
			ATLASSERT(SUCCEEDED(hr));

			devices.push_back(wstring(friendlyName));
		}

		spMoniker.Release();
	}
}

HRESULT DirectShow::EnumDevices(REFGUID category, IEnumMoniker **ppEnum)
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

vector<wstring> DirectShow::ListVideoInputDevices()
{
	vector<wstring> devices;

	CComPtr<IEnumMoniker> spEnum;
	HRESULT hr = EnumDevices(CLSID_VideoInputDeviceCategory, &spEnum);
	if (SUCCEEDED(hr))
	{
		ReadDeviceInformation(spEnum, devices);
	}

	return devices;
}
