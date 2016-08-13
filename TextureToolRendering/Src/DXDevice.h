#pragma once
FORWARD_DECLARE_CLASS(RenderView);
class DXDevice
{
public:
	DXDevice();
	~DXDevice();
	
	// Initialize the device
	static HRESULT				InitializeDevice();

	static ID3D11Device*        GetDevice() { return mDevice; }

	static ID3D11DeviceContext* GetContext() { return mDeviceContext; }

protected:
	// The device
	static ID3D11Device*        mDevice;
	// The device context
	static ID3D11DeviceContext* mDeviceContext;

};

