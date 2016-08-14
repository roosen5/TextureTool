#pragma once
class Shader
{
public:
	Shader();
	~Shader();

	void                      SetShader(ID3D11DeviceChild* pShader) { mShader = pShader; }
	const ID3D11DeviceChild*  GetShader() const { return mShader; }

	void                      SetBlob(ID3DBlob* pShaderBlob) { mShaderBlob = pShaderBlob; }
	ID3DBlob*                 GetBlob() { return mShaderBlob; }

	void                      SetShaderName(const wchar_t* pName) { mName = pName; }
	std::wstring              GetShaderName() { return mName; }

private:
	std::wstring              mName;
	ID3D11DeviceChild*        mShader;
	ID3DBlob*                 mShaderBlob;
};

