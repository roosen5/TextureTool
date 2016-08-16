#include "TT_PCH.h"
#include "Texture.h"


Texture::Texture():
	m2DTexture(nullptr),
	mShaderResourceView(nullptr)
{
}


Texture::~Texture()
{
	for (int i = 0; i < mSurfaceList.size(); i++)
	{
		delete mSurfaceList[i];
	}
	mSurfaceList.clear();
	SAFE_RELEASE(m2DTexture);
	SAFE_RELEASE(mShaderResourceView);
}

void Texture::Set2DTexture(ID3D11Texture2D* pTexture2D)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	pTexture2D->GetDesc(&textureDesc);
	mFormat = textureDesc.Format;
	mWidth = textureDesc.Width;
	mHeight = textureDesc.Height;
	m2DTexture = pTexture2D;
}

const Surface* Texture::GetSurface(int pIndex) const
{
	assert(pIndex < mSurfaceList.size());
	return mSurfaceList[pIndex];
}