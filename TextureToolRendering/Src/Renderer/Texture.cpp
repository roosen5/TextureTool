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