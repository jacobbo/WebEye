#ifndef WEBEYE_DIRECTSHOW_H
#define WEBEYE_DIRECTSHOW_H

#include <string>
#include <vector>

#define _ATL_NO_AUTOMATIC_NAMESPACE
#include <atlbase.h>

namespace WebEye
{
	class DirectShow
	{
	public:
		std::vector<std::wstring> ListVideoInputDevices();

	private:
		void ReadDeviceInformation(IEnumMoniker *pEnum, std::vector<std::wstring> &devices);
		HRESULT EnumDevices(REFGUID category, IEnumMoniker **ppEnum);
	};
}

#endif // WEBEYE_DIRECTSHOW_H
