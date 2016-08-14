#include "TT_PCH.h"


Shader::Shader():
	mShader(nullptr),
	mShaderBlob(nullptr)
{
}


Shader::~Shader()
{
	SAFE_RELEASE(mShader);
	SAFE_RELEASE(mShaderBlob);
}

