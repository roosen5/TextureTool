#include "TT_PCH.h"


TextureConverter::TextureConverter()
{
}


TextureConverter::~TextureConverter()
{
}

HRESULT TextureConverter::Convert(Texture*& pOutTexture)
{
	assert(pOutTexture == nullptr);
	pOutTexture = new Texture();

	// Create texture
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = mTexture->GetWidth();
	textureDesc.Height = mTexture->GetHeight();
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	HRESULT result;

	Surface* surface = new Surface();
	surface->Assign(mTexture->GetFirstSurface());

	// Compress surface here //

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = surface->GetData();
	initData.SysMemPitch = surface->GetRowPitch();
	initData.SysMemSlicePitch = surface->GetDepthPitch();


	ID3D11Texture2D* dx2DTexture;
	result = DXDevice::GetDevice()->CreateTexture2D(&textureDesc, &initData, &dx2DTexture);
	if FAILED(result)
	{
		return result;
	}
	pOutTexture->Set2DTexture(dx2DTexture);

	ID3D11ShaderResourceView* surfaceResourceView;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	memset(&shaderResourceViewDesc, 0, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	result = DXDevice::GetDevice()->CreateShaderResourceView(dx2DTexture, &shaderResourceViewDesc, &surfaceResourceView);


	pOutTexture->AddSurface(surface);
	pOutTexture->SetResourceView(surfaceResourceView);

	return S_OK;
}

