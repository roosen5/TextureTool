#include "TT_PCH.h"

Surface::Surface():
	mData(nullptr)
{
	Setup(0, 0, 0, 0);
}

Surface::Surface(const char* pFileName):
	mData(nullptr)
{
	LoadFromFile(pFileName);
}

Surface::~Surface()
{
	Free();
}


void Surface::Setup(unsigned int pWidth, unsigned int pHeight, uint pRowPitch, size_t pDepthPitch)
{
	mWidth = pWidth;
	mHeight = pHeight;
	mData = (BYTE*)malloc(pDepthPitch);
	mRowPitch = pRowPitch;
	mDepthPitch = pDepthPitch;
}

void Surface::Assign(const Surface* pSurface)
{
	mWidth = pSurface->GetWidth();
	mHeight = pSurface->GetHeight();
	mRowPitch = pSurface->GetRowPitch();
	mDepthPitch = pSurface->GetDepthPitch();
	mFormat = pSurface->GetFormat();
	mData = (BYTE*)malloc(pSurface->GetDepthPitch());
	if (mFormat == pSurface->GetFormat())
	{
		memcpy(mData, pSurface->GetData(), mDepthPitch);
	}
}

void Surface::LoadFromFile(const char* pFileName)
{
	QImage image;
	image.load(pFileName);
	if (image.isNull())
		return;
	// Just convert 1 channel images to 3 channel
	if (image.allGray())
	{
		image = image.convertToFormat(QImage::Format::Format_RGB32);
	}
	image = image.rgbSwapped(); //MATHIJS Explain?
	Setup(image.width(), image.height(), image.bytesPerLine(), image.byteCount());
	assert(mDepthPitch == image.byteCount());
	memcpy(mData, image.bits(), mDepthPitch);
	mFormat= QTFromImageFormat(image.format());
}

void Surface::NormalizeColors() 
{
	QRgb* colorData = (QRgb*)mData;
	for (int x = 0; x < mWidth; x++)
	for (int y = 0; y < mHeight; y++)
	{
		QColor color((QRgb)colorData[x + y*mWidth]);
		const float red = color.redF();
		const float green = color.greenF();
		const float blue = color.blueF();
		// Get the length/magnitude of the color
		const float length = sqrt((red * red) + (green * green) + (blue * blue));
		// And devide the other colors with it
		color.setRedF(red / length);
		color.setGreenF(green / length);
		color.setBlueF(blue / length);
		colorData[x + y * mWidth] = color.rgb();
	}
}

void Surface::Free()
{
	SAFE_DELETE(mData);
	Setup(0, 0, 0, 0);
}

