#pragma once
typedef std::vector<Surface*> SurfaceList;
class Surface
{
public:
	Surface();
	Surface(const char* pFileName);
	~Surface();

	void              Setup(unsigned int pWidth, unsigned int pHeight, uint pRowPitch, size_t pDepthPitch);

	void              Assign(const Surface* pSurface);

	bool              IsValid() { return mData != nullptr; }

	void              LoadFromFile(const char* pFileName);

	unsigned int      GetWidth() const { return mWidth; }

	unsigned int      GetHeight() const{ return mHeight; }

	int               GetRowPitch() const { return mRowPitch; }

	int               GetDepthPitch() const { return mDepthPitch; }

	const BYTE*       GetData() const { return mData; }

	BYTE*             GetData() { return mData; }

	void              SetFormat(DXGI_FORMAT pFormat) { mFormat = pFormat; }

	DXGI_FORMAT       GetFormat() { return mFormat; }

	const DXGI_FORMAT GetFormat() const { return mFormat; }

	void              NormalizeColors();

	void              Free();

private:
	BYTE*			  mData; // The pixel bits

	unsigned int      mRowPitch;

	unsigned int      mDepthPitch;
	// Width and height
	unsigned int      mWidth;
	unsigned int      mHeight;

	DXGI_FORMAT       mFormat;
};

