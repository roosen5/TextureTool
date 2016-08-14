#pragma once
FORWARD_DECLARE_CLASS(Surface);
class Texture
{
public:
	Texture();
	~Texture();
	void                                SetResourceView(ID3D11ShaderResourceView* pShaderResourceView) { mShaderResourceView = pShaderResourceView; }
	ID3D11ShaderResourceView*			GetResourceView() { return mShaderResourceView; }
	const ID3D11ShaderResourceView*     GetResourceView() const { return mShaderResourceView; }

	void                                Set2DTexture(ID3D11Texture2D* pTexture2D) { m2DTexture = pTexture2D; }
	const ID3D11Texture2D*              Get2DTexture() { return m2DTexture; }

	void                                AddSurface(Surface* pSurface) { mSurfaceList.push_back(pSurface); }
	const Surface*                      GetSurface(int pIndex) const { return mSurfaceList[pIndex]; }

	void                                SetFileName(const char* pFileName) { mFileName = pFileName; }
	QString                             GetFileName() { return mFileName; }

private:
	// The imported file path
	QString                             mFileName;

	// The list of surfaces 
	std::vector<Surface*>               mSurfaceList;

	// The actual DirectX texture2D resource 
	ID3D11Texture2D*                    m2DTexture;

	// The shader resource view of the texture
	ID3D11ShaderResourceView*           mShaderResourceView;
};

