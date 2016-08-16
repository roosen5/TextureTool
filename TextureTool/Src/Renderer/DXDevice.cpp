#include "TT_PCH.h"
#pragma hdrstop
#include "DXDevice.h"



ID3D11Device*        DXDevice::mDevice       = nullptr;
ID3D11DeviceContext* DXDevice::mDeviceContext= nullptr;


DXDevice::DXDevice()
{
}


DXDevice::~DXDevice()
{
}

HRESULT DXDevice::InitializeDevice()
{
	UINT createDeviceFlags = 0;
	// Only use debug flag when running in debug mode
#if _DEBUG
	createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	// These are the feature levels that we will accept.
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// This will be the feature level that 
	// is used to create our device and swap chain.
	D3D_FEATURE_LEVEL featureLevel;


	HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE,
		nullptr, createDeviceFlags, featureLevels, _countof(featureLevels),
		D3D11_SDK_VERSION, &mDevice, &featureLevel, &mDeviceContext);

	if FAILED(result)
	{
		return result;
	}
	return result;
}

void DXDevice::ShutDown()
{
	SAFE_RELEASE(mDevice);
	SAFE_RELEASE(mDeviceContext);
}
