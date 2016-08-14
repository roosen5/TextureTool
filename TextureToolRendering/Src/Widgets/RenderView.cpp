#include "TT_PCH.h"
#pragma hdrstop
#include "RenderView.h"

RenderView::RenderView(QWidget* pWidget) : QWidget::QWidget(pWidget)
{
	setLayout(new QVBoxLayout(this));
	layout()->addWidget(new QComboBox(this));
    mRenderTargetView   = nullptr;
    mRasterizerState    = nullptr;
	mSwapChain = nullptr;

// Disable updates, because updates would clear the screen automatically
	setUpdatesEnabled(false);
}


RenderView::~RenderView()
{
	SAFE_RELEASE(mRenderTargetView);
	SAFE_RELEASE(mRasterizerState);
	SAFE_RELEASE(mSwapChain);
	SAFE_RELEASE(mSamplerState);
}

void RenderView::Render2DTexture(const Texture* pTexture)
{
	Model model;

	const Material* material = model.GetMaterial();

	ID3D11DeviceContext* Context = DXDevice::GetContext();

	const UINT vertexStride = sizeof(Vertex);
	const UINT offset = 0;


	ID3D11VertexShader* vertexShader = (ID3D11VertexShader*)material->GetVertexShader()->GetShader();
	Context->VSSetShader(vertexShader, nullptr, 0);

	ID3D11PixelShader* pixelShader = (ID3D11PixelShader*)material->GetPixelShader()->GetShader();
	Context->PSSetShader(pixelShader, nullptr, 0);

	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width();
	viewport.Height = height();

	Context->RSSetState(GetRasterizerState());
	Context->RSSetViewports(1, &viewport);


	ID3D11Buffer* vertexBuffer = model.GetVertexBuffer();
	Context->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &offset);
	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Context->IASetInputLayout((ID3D11InputLayout*)material->GetInputLayout());

	ID3D11ShaderResourceView* shaderResourceView = (ID3D11ShaderResourceView*)pTexture->GetResourceView();
	Context->PSSetShaderResources(0, 1, &shaderResourceView);
	Context->VSSetShaderResources(0, 1, &shaderResourceView);


	Context->OMSetRenderTargets(1, &mRenderTargetView, nullptr);

	Context->Draw(6, 0);
}

void RenderView::OnResized()
{
	if (mSwapChain == nullptr)
	{
		// If there's no swap chain, return, no need to resize if there's nothing
		return;
	}
	// Resize the swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	mSwapChain->GetDesc(&swapChainDesc);

	// Release all the buffers associated with the swapchain
	SAFE_RELEASE(mRenderTargetView);

	// Resize the swap chain
	HRESULT result= mSwapChain->ResizeBuffers(swapChainDesc.BufferCount, width(), height(), swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);

	if FAILED(result)
	{
		ShowError(result, "mSwapChain->ResizeBuffers");
	}

	// Recreate the renderTargetView from the backbuffer
	ID3D11Texture2D* backBuffer;
	result = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);

	if FAILED(result)
	{
		ShowError(result, "mSwapChain->GetBuffer");
	}

	ID3D11RenderTargetView* renderTargetView = nullptr;
	result = DXDevice::GetDevice()->CreateRenderTargetView(backBuffer, nullptr, &mRenderTargetView);

	if FAILED(result)
	{
		ShowError(result, "DXDevice::GetDevice()->CreateRenderTargetView");
	}

	SAFE_RELEASE(backBuffer);
}

void RenderView::resizeEvent(QResizeEvent* pEvent)
{
	Base::resizeEvent(pEvent);
	OnResized();
}

HRESULT RenderView::Initialize()
{
	// Create the swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	// We only need one buffer
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width();
	swapChainDesc.BufferDesc.Height = height();
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Take the refreshrate from the primary screen (Should be 50/60)
	swapChainDesc.BufferDesc.RefreshRate.Numerator = QGuiApplication::primaryScreen()->refreshRate();
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	// Obtain the widgets window id and cast it to a HWND
	swapChainDesc.OutputWindow = GetWindowHandle(); 

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

	// Setup the sampler
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;

	result = DXDevice::GetDevice()->CreateSamplerState(&samplerDesc, &mSamplerState);
	DXDevice::GetContext()->PSSetSamplers(0, 1, &mSamplerState);

	return result;
}

void RenderView::Clear(const QColor& pClearColor)
{
	FLOAT clearColors[4];
	clearColors[0] = pClearColor.redF();
	clearColors[1] = pClearColor.greenF();
	clearColors[2] = pClearColor.blueF();
	clearColors[3] = 1;
	DXDevice::GetContext()->ClearRenderTargetView(mRenderTargetView, clearColors);
}

void RenderView::Blit()
{
	//printf("\n" __FUNCTION__);
	mSwapChain->Present((UINT)1, 0);
}
