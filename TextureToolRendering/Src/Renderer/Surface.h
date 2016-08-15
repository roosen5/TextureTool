#pragma once
class Surface
{
public:
	Surface();
	Surface(const char* pFileName);
	~Surface();

	void          Assign(const Surface* pSurface);

	void          LoadFromFile(const char* pFileName);

	unsigned int  GetWidth() const { return mImage.width(); }

	unsigned int  GetHeight() const{ return mImage.height(); }

	int           GetRowPitch() const { return mImage.bytesPerLine(); }

	int           GetDepthPitch() const { return mImage.byteCount(); }

	const BYTE*   GetData() const { return mImage.bits(); }

	const QImage* GetQImage() const { return &mImage; }

	void          FlipRGB() { mImage = mImage.rgbSwapped(); }
private:
	// The QImage, where the image bits are stored
	QImage        mImage;
};

