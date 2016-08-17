#include "TT_PCH.h"

RenderView::RenderView(QWidget* pWidget) : 
	QWidget::QWidget(pWidget), 
	mTexturePreviewModel(nullptr),
	mRenderTargetView(nullptr),
	mRasterizerState(nullptr),
	mSwapChain(nullptr),
	mBlendState(nullptr),
	mRenderChannels(0xFFFFFFFF),
	mForcedMipmap(MIPMAP_AUTO),
	mGrayScaleOutput(false)
{
	
	mTexturePreviewModel = new Model();

	setLayout(new QVBoxLayout(this));
	layout()->addWidget(new QComboBox(this));

// Disable updates, because updates would clear the screen automatically
	setUpdatesEnabled(false);
}


RenderView::~RenderView()
{
	SAFE_RELEASE(mRenderTargetView);
	SAFE_RELEASE(mRasterizerState);
	SAFE_RELEASE(mSwapChain);
	SAFE_RELEASE(mSamplerState);
	SAFE_RELEASE(mBlendState);
	SAFE_DELETE(mTexturePreviewModel);
}

void RenderView::Render2DTexture(const Texture* pTexture)
{
	TexturePreviewMaterial* material = (TexturePreviewMaterial*)mTexturePreviewModel->GetMaterial();

	TexturePreviewInfo info = material->GetTexturePreviewInfo();
	info.mForceMip		 = mForcedMipmap;
	info.mRenderChannels = mRenderChannels;
	info.mGrayScaleOutput= mGrayScaleOutput;
	material->SetTexturePreviewInfo(info);

	ID3D11DeviceContext* context = DXDevice::GetContext();

	const UINT vertexStride = sizeof(Vertex);
	const UINT offset = 0;


	ID3D11VertexShader* vertexShader = (ID3D11VertexShader*)material->GetVertexShader()->GetShader();
	context->VSSetShader(vertexShader, nullptr, 0);

	ID3D11PixelShader* pixelShader = (ID3D11PixelShader*)material->GetPixelShader()->GetShader();
	context->PSSetShader(pixelShader, nullptr, 0);


	ID3D11Buffer* texturePreviewBuffer = (ID3D11Buffer*)material->GetTexturePreviewInfoBuffer();

	context->PSSetConstantBuffers(0, 1, &texturePreviewBuffer);

	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width();
	viewport.Height = height();

	context->RSSetState(GetRasterizerState());
	context->RSSetViewports(1, &viewport);


	ID3D11Buffer* vertexBuffer = mTexturePreviewModel->GetVertexBuffer();
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout((ID3D11InputLayout*)material->GetInputLayout());

	ID3D11ShaderResourceView* shaderResourceView = (ID3D11ShaderResourceView*)pTexture->GetResourceView();

	context->PSSetShaderResources(0, 1, &shaderResourceView);
	context->VSSetShaderResources(0, 1, &shaderResourceView);

	context->OMSetBlendState(mBlendState, nullptr, 0xffffff);

	context->OMSetRenderTargets(1, &mRenderTargetView, nullptr);

	context->Draw(6, 0);
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
	setFixedHeight(width());
	OnResized();
	emit Resized();
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
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;

	result = DXDevice::GetDevice()->CreateSamplerState(&samplerDesc, &mSamplerState);
	if FAILED(result)
		return result;
	DXDevice::GetContext()->PSSetSamplers(0, 1, &mSamplerState);

	// Setup blending state, to make sure we're able to render alpha
	D3D11_BLEND_DESC blendState;
	ZeroMemory(&blendState, sizeof(D3D11_BLEND_DESC));
	blendState.AlphaToCoverageEnable = 0;
	blendState.IndependentBlendEnable = 0;
	blendState.RenderTarget[0].BlendEnable = 1;
	blendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendState.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	result = DXDevice::GetDevice()->CreateBlendState(&blendState, &mBlendState);

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

void RenderView::SetCurrentPixelShader(const wchar_t* pShaderName)
{
	TexturePreviewMaterial* material = mTexturePreviewModel->GetMaterial();
	material->ReleasePixelShader();
	material->SetPixelShader(ShaderManager::LoadShader(pShaderName));
}

const Shader* RenderView::GetCurrentPixelShader()
{
	if (mTexturePreviewModel != nullptr)
	{
		return mTexturePreviewModel->GetMaterial()->GetPixelShader();
	}
	return nullptr;
}

void RenderView::SetCurrentVertexShader(const wchar_t* pShaderName)
{
	TexturePreviewMaterial* material = mTexturePreviewModel->GetMaterial();
	material->ReleaseVertexShader();
	material->SetVertexShader(ShaderManager::LoadShader(pShaderName));
}

const Shader* RenderView::GetCurrentVertexShader()
{
	if (mTexturePreviewModel != nullptr)
	{
		return mTexturePreviewModel->GetMaterial()->GetVertexShader();
	}
	return nullptr;
}

void RenderView::Blit()
{
	//printf("\n" __FUNCTION__);
	mSwapChain->Present((UINT)1, 0);
}

