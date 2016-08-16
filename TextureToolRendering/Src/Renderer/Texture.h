#pragma once
FORWARD_DECLARE_CLASS(Surface);
class Texture
{
public:
	Texture();
	~Texture();

	// Returns the texture format
	DXGI_FORMAT                         GetFormat() { return mFormat ;}

	// Set/get the directX resourceview, which is sent to the shader
	void                                SetResourceView(ID3D11ShaderResourceView* pShaderResourceView) { mShaderResourceView = pShaderResourceView; }
	ID3D11ShaderResourceView*			GetResourceView() { return mShaderResourceView; }
	const ID3D11ShaderResourceView*     GetResourceView() const { return mShaderResourceView; }

	// Set/get the directX texture
	void                                Set2DTexture(ID3D11Texture2D* pTexture2D);
	const ID3D11Texture2D*              Get2DTexture() { return m2DTexture; }

	// Append surface to the surface list
	void                                AddSurface(Surface* pSurface) { mSurfaceList.push_back(pSurface); }
	const Surface*                      GetSurface(int pIndex) const;

	Surface*                            GetFirstSurface() { return mSurfaceList[0]; }
	const Surface*                      GetFirstSurface() const { return mSurfaceList[0]; }

	// Also the mipmap count
	unsigned int                        GetSurfaceCount() { return mSurfaceList.size(); }

	void                                SetFileName(const char* pFileName) { mFileName = pFileName; }
	QString                             GetFileName() { return mFileName; }


private:
	// Sizes
	unsigned int                        mHeight;

	unsigned int                        mWidth;

	// The current format of the texture
	DXGI_FORMAT                         mFormat;

	// The imported file path
	QString                             mFileName;

	// The list of surfaces 
	std::vector<Surface*>               mSurfaceList;

	// The actual DirectX texture2D resource 
	ID3D11Texture2D*                    m2DTexture;

	// The shader resource view of the texture
	ID3D11ShaderResourceView*           mShaderResourceView;
};

