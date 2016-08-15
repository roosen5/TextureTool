#include "TT_PCH.h"

Surface::Surface()
{

}

Surface::Surface(const char* pFileName)
{
	LoadFromFile(pFileName);
}

Surface::~Surface()
{
}

void Surface::Assign(const Surface* pSurface)
{
	mImage = *pSurface->GetQImage();
}

void Surface::LoadFromFile(const char* pFileName)
{
	mImage.load(pFileName);
	mImage = mImage.rgbSwapped();
}
