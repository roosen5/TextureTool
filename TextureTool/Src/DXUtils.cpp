#include "TT_PCH.h"

bool DXIsBlockCompressed(DXGI_FORMAT pFormat)
{
	switch (pFormat)
	{
	case DXGI_FORMAT_BC1_TYPELESS:
		return true;
	case DXGI_FORMAT_BC1_UNORM:
		return true;
	case DXGI_FORMAT_BC1_UNORM_SRGB:
		return true;
	case DXGI_FORMAT_BC2_TYPELESS:
		return true;
	case DXGI_FORMAT_BC2_UNORM:
		return true;
	case DXGI_FORMAT_BC2_UNORM_SRGB:
		return true;
	case DXGI_FORMAT_BC3_TYPELESS:
		return true;
	case DXGI_FORMAT_BC3_UNORM:
		return true;
	case DXGI_FORMAT_BC3_UNORM_SRGB:
		return true;
	case DXGI_FORMAT_BC4_TYPELESS:
		return true;
	case DXGI_FORMAT_BC4_UNORM:
		return true;
	case DXGI_FORMAT_BC4_SNORM:
		return true;
	case DXGI_FORMAT_BC5_TYPELESS:
		return true;
	case DXGI_FORMAT_BC5_UNORM:
		return true;
	case DXGI_FORMAT_BC5_SNORM:
		return true;
	default:
		return false;
	}
	return false;
}
