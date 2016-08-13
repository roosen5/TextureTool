#include "TT_PCH.h"
#pragma hdrstop
#include "RenderInterface.h"

RenderInterface::RenderInterface(RenderView* pRenderView)
{
	mRenderView			= nullptr;
    mRenderTargetView   = nullptr;
    mRasterizerState    = nullptr;

	mRenderView = pRenderView;
	connect(mRenderView, SIGNAL(WMPaintRequested()), SLOT(OnViewWMPaintRequested()));
}


RenderInterface::~RenderInterface()
{
}

void RenderInterface::Render2DTexture()
{

}

void RenderInterface::OnViewWMPaintRequested()
{
	Blit();
}

bool RenderInterface::nativeEvent(const QByteArray& pEventType, void *pMsg, long *pResult)
{
	MSG* message = static_cast<MSG*>(pMsg);
	if (message->message == WM_PAINT) // Catch windows paint event
	{
		Blit();
	}
	return true;
}

HRESULT RenderInterface::InitializeDirectX()
{
	// Create the swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	// We only need one buffer
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = mRenderView->width();
	swapChainDesc.BufferDesc.Height = mRenderView->height();
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Take the refreshrate from the primary screen (Should be 50/60)
	swapChainDesc.BufferDesc.RefreshRate.Numerator = QGuiApplication::primaryScreen()->refreshRate();
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	// Obtain the widgets window id and cast it to a HWND
	swapChainDesc.OutputWindow = mRenderView->GetWindowHandle(); // MATHIJS MAKE NICE

	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Windowed = TRUE;


	IDXGIFactory* factory = NULL;
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory)))
	{
		return false;
	}

	HRESULT result = factory->CreateSwapChain(DXDevice::GetDevice(), &swapChainDesc, &mSwapChain);

	if FAILED(result)
	{
		return result;
	}


	// Now we need to initialize the buffers of the swap chain.
	// Next initialize the back buffer of the swap chain and associate it to a 
	// render target view.
	ID3D11Texture2D* backBuffer;
	result = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);

	if FAILED(result)
	{
		return result;
	}

	ID3D11RenderTargetView* renderTargetView = nullptr;
	result = DXDevice::GetDevice()->CreateRenderTargetView(backBuffer, nullptr, &mRenderTargetView);
	if FAILED(result)
	{
		return result;
	}

	SAFE_RELEASE(backBuffer);

	// Setup rasterizer.
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state object.
	result = DXDevice::GetDevice()->CreateRasterizerState(&rasterizerDesc, &mRasterizerState);

	return result;
}

void RenderInterface::Clear(const QColor& pClearColor)
{
	FLOAT clearColors[4];
	clearColors[0] = pClearColor.redF();
	clearColors[1] = pClearColor.greenF();
	clearColors[2] = pClearColor.blueF();
	clearColors[3] = 1;
	DXDevice::GetContext()->ClearRenderTargetView(mRenderTargetView, clearColors);
}

void RenderInterface::Blit()
{
	mSwapChain->Present((UINT)1, 0);
}

