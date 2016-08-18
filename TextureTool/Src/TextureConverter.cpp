#include "TT_PCH.h"



TextureConverter::TextureConverter() :mGenerateMipmaps(false)
{
}


TextureConverter::~TextureConverter()
{
}

void TextureConverter::CopyDataToStratchImage(DirectX::ScratchImage* pScratchImage, const Surface* pSurface)
{
	const BYTE* srcBuffer = pSurface->GetData();
	BYTE* dstBuffer = pScratchImage->GetPixels();
	size_t size = pSurface->GetDepthPitch();
	memcpy(dstBuffer, srcBuffer, size);
}

void TextureConverter::CopyDataToSurface(Surface* pSurface, const DirectX::ScratchImage* pScratchImage)
{
	const BYTE* srcBuffer = pScratchImage->GetPixels();
	BYTE* dstBuffer = pSurface->GetData();
	size_t size = pScratchImage->GetPixelsSize();
	memcpy(dstBuffer, srcBuffer, size);
}

void TextureConverter::CopyDataToSurface(Surface* pSurface, const DirectX::Image* pImage)
{
	const BYTE* srcBuffer = pImage->pixels;
	BYTE* dstBuffer = pSurface->GetData();
	size_t size = pImage->slicePitch;
	memcpy(dstBuffer, srcBuffer, size);
}

HRESULT TextureConverter::Compress(DXGI_FORMAT pFormat, SurfaceList& pSurfaceList)
{
	for (int mipmapIndex = 0; mipmapIndex < pSurfaceList.size(); mipmapIndex++)
	{
		Surface* surface = pSurfaceList[mipmapIndex];

		if (pFormat == DXGI_FORMAT_BC5_UNORM||pFormat == DXGI_FORMAT_BC5_SNORM)
		{
			// If its supposed to be used as a normal map, generating the third/green axis
			// The magnitude of RGB should be 1, this is to make sure its normalized 
			surface->NormalizeColors();
		}

		// Copy the source surface onto the scratchImage
		DirectX::ScratchImage scratchImage;
		HRESULT result = scratchImage.Initialize2D(surface->GetFormat(), surface->GetWidth(), surface->GetHeight(), 1, 1);
		if FAILED(result)
		{
			return result;
		}
		CopyDataToStratchImage(&scratchImage, surface);

		DirectX::ScratchImage compressedScratchImage;
		// Compress 
		result = DirectX::Compress(*scratchImage.GetImages(), pFormat, DirectX::TEX_COMPRESS_DEFAULT, 0.0f, compressedScratchImage);
		if FAILED(result)
		{
			return result;
		}

		surface->Free();

		surface->Setup(compressedScratchImage.GetImages()->width, compressedScratchImage.GetImages()->height, 
					   compressedScratchImage.GetImages()->rowPitch, compressedScratchImage.GetImages()->slicePitch);

		surface->SetFormat(pFormat);
		CopyDataToSurface(surface, compressedScratchImage.GetImages());
		pSurfaceList[mipmapIndex] = surface;
	}
	return S_OK;
}

HRESULT TextureConverter::GenerateMipmaps(const Surface* pSrcSurface, SurfaceList& pSurfaceList)
{
	// Copy the source surface onto the scratchImage
	DirectX::ScratchImage scratchImage;
	scratchImage.Initialize2D(pSrcSurface->GetFormat(), pSrcSurface->GetWidth(), pSrcSurface->GetHeight(), 1, 1);

	CopyDataToStratchImage(&scratchImage, pSrcSurface);

	DirectX::ScratchImage mipChain;

	// Generate the mipmaps onto the mipChain object
	HRESULT result = DirectX::GenerateMipMaps(*scratchImage.GetImages(), 0, 0, mipChain);

	for (int i = 0; i < mipChain.GetImageCount(); i++)
	{
		Surface* surface = new Surface();
		const DirectX::Image* image = mipChain.GetImage(i, 0, 0);

		surface->Setup(image->width, image->height, image->rowPitch, image->slicePitch);
		surface->SetFormat(pSrcSurface->GetFormat());
		CopyDataToSurface(surface, image);

		pSurfaceList.push_back(surface);
	}

	return result;
}

HRESULT TextureConverter::Process(Texture*& pOutTexture)
{
	assert(pOutTexture == nullptr);
	pOutTexture = new Texture();

	SurfaceList surfaceList;

	// First generate the mipmaps, as the mipmap generator does not work with block compressed images
	HRESULT result = S_OK;
	if (mGenerateMipmaps)
	{
		result = GenerateMipmaps(mSrcTexture->GetFirstSurface(), surfaceList);
	}
	else
	{
		Surface* surface = new Surface();
		surface->Assign(mSrcTexture->GetFirstSurface());
		surfaceList.push_back(surface);
	}

	if (mSharpeningFilter->GetSharpeningEnabled())
	{
		int surfaceIndex = 0;
		if (mSharpeningFilter->GetOnlyApplyOnMipmaps())
		{
			surfaceIndex++;
		}
		for (; surfaceIndex < surfaceList.size(); surfaceIndex++)
		{
			mSharpeningFilter->ApplyKernelToSurface(surfaceList[surfaceIndex]);
		}
	}

	if (mDstFormat != mSrcTexture->GetFirstSurface()->GetFormat()) // Don't do anything if its already the desired type
	{
		result = Compress(mDstFormat, surfaceList);
	}

	if FAILED(result)
	{
		return result;
	}

	Surface* firstMip = surfaceList[0];

	int mipmapCount = surfaceList.size();

	// Create the 2Dtexture
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = firstMip->GetWidth();
	textureDesc.Height = firstMip->GetHeight();
	textureDesc.MipLevels = mipmapCount;
	textureDesc.ArraySize = 1;
	textureDesc.Format = mDstFormat;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA* initData = new D3D11_SUBRESOURCE_DATA[mipmapCount];
	// Create initData for every mipmap
	for (int i = 0; i < mipmapCount; i++)
	{
		// Assign all the initialization data's 
		initData[i].pSysMem = surfaceList[i]->GetData();
		initData[i].SysMemPitch = surfaceList[i]->GetRowPitch();
		initData[i].SysMemSlicePitch = surfaceList[i]->GetDepthPitch();

		// Add the surface to the texture
		pOutTexture->AddSurface(surfaceList[i]);
	}

	ID3D11Texture2D* dx2DTexture;
	result = DXDevice::GetDevice()->CreateTexture2D(&textureDesc, initData, &dx2DTexture);
	if FAILED(result)
	{
		return result;
	}

	pOutTexture->Set2DTexture(dx2DTexture);

	ID3D11ShaderResourceView* surfaceResourceView;

	// Create the resourceview
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	memset(&shaderResourceViewDesc, 0, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = mipmapCount;

	result = DXDevice::GetDevice()->CreateShaderResourceView(dx2DTexture, &shaderResourceViewDesc, &surfaceResourceView);


	pOutTexture->SetResourceView(surfaceResourceView);

	return S_OK;
}

