#pragma once
class TextureConverter
{
public:
	TextureConverter();
	~TextureConverter();
	// Do all generations and compressions
	HRESULT           Process(Texture*& pOutTexture);

	// Copy data from surface and place it into scratch image
	void	          CopyDataToStratchImage(DirectX::ScratchImage* pScratchImage, const Surface* pSurface);
	void	          CopyDataToStratchImage(DirectX::Image* pScratchImage, const Surface* pSurface);

	// Copy data from scratch image and place it into surface 
	void	          CopyDataToSurface(Surface* pSurface, const DirectX::ScratchImage* pScratchImage);
	void	          CopyDataToSurface(Surface* pSurface, const DirectX::Image* pScratchImage);

	// Compress function
	HRESULT           Compress(DXGI_FORMAT pFormat, SurfaceList& pSurfaceList);

	// Generates the most possible mipmaps
	HRESULT           GenerateMipmaps(const Surface* pSurface, SurfaceList& pSurfaceList);

	// Binds the original texture
	void              SetOriginalTexture(Texture* pTexture) { mSrcTexture = pTexture; }

	// Exports the mSrcTexture to a dds file
	HRESULT           SaveToFile(const wchar_t* pFileName);

	// Sets the format
	void              SetDstFormat(DXGI_FORMAT pFormat) { mDstFormat = pFormat; }

	// Even set if there's no sharpening filter
	void              SetSharpeningFilter(SharpeningFilter* pSharpeningFilter) { mSharpeningFilter = pSharpeningFilter; }

	void              SetGenerateMipmaps(bool pGenerateMipmaps) { mGenerateMipmaps = pGenerateMipmaps; }

private:
 //  The texture we will be using as reference to convert
	Texture*          mSrcTexture;

 //  The format we want to convert to 
	DXGI_FORMAT       mDstFormat;

 //  The sharpeningfilter has to be applied in the Process function
	SharpeningFilter* mSharpeningFilter;

 //  Specifies if we want to generate mipmaps in the Process function
	bool              mGenerateMipmaps;

};

