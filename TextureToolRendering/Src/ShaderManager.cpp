#include "TT_PCH.h"


ShaderManager::ShaderManager()
{
}


ShaderManager::~ShaderManager()
{
}

ShaderType ShaderManager::RetrieveShaderType(const wchar_t* pShaderName)
{
	if (wcsncmp(pShaderName + wcsnlen(pShaderName, MAX_PATH) - 2, L"VS", 2)==0)
	{
		return stVertexShader;
	}

	if (wcsncmp(pShaderName + wcsnlen(pShaderName, MAX_PATH) - 2, L"PS", 2)==0)
	{
		return stPixelShader;
	}
	return stUnknownShader;
}

HRESULT ShaderManager::CreatePixelShader(const wchar_t* pShaderName, ID3D11PixelShader*& pPixelShader, ID3DBlob*& pPixelShaderBlob)
{
	std::wstring shaderName = pShaderName + std::wstring(L".cso");
	HRESULT result = D3DReadFileToBlob(shaderName.c_str(), &pPixelShaderBlob);
	if (result == S_OK)
	{
		DXDevice::GetDevice()->CreatePixelShader(pPixelShaderBlob->GetBufferPointer(), pPixelShaderBlob->GetBufferSize(), nullptr, &pPixelShader);
	}
	return result;
}

HRESULT ShaderManager::CreateVertexShader(const wchar_t* pShaderName, ID3D11VertexShader*& pVertexShader, ID3DBlob*& pVertexShaderBlob)
{
	std::wstring shaderName = pShaderName + std::wstring(L".cso");
	HRESULT result = D3DReadFileToBlob(shaderName.c_str(), &pVertexShaderBlob);
	if (result == S_OK)
	{
		DXDevice::GetDevice()->CreateVertexShader(pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), nullptr, &pVertexShader);
	}
	return result;
}


Shader* ShaderManager::LoadShader(const wchar_t* pShaderName)
{
	ShaderEntry& shaderEntry = mShaderList[pShaderName];
	if(shaderEntry.ref==0)
	{
		Shader* shader = new Shader();
		ID3D11DeviceChild* dxshaderPtr=nullptr;
		ID3D10Blob* dxshaderBlob=nullptr;
		HRESULT result = SEC_E_OK;
		switch (RetrieveShaderType(pShaderName))
		{
		case stPixelShader:
			result = CreatePixelShader(pShaderName, (ID3D11PixelShader*&)dxshaderPtr, dxshaderBlob);
			break;

		case stVertexShader:
			result = CreateVertexShader(pShaderName, (ID3D11VertexShader*&)dxshaderPtr, (ID3D10Blob*)dxshaderBlob);
			break;
		default:
			result = E_UNEXPECTED;
			break;
		}
		if FAILED(result)
		{
			ShowError(result, "ShaderManager::LoadShader");
		}

		shader->SetShaderName(pShaderName);
		shader->SetShader(dxshaderPtr);
		shader->SetBlob(dxshaderBlob);
		shaderEntry.shader = shader;
	}
	shaderEntry.ref++;
	return shaderEntry.shader;
}

void ShaderManager::UnloadShader(const wchar_t* pShaderName)
{
	ShaderEntry& shaderEntry = mShaderList[pShaderName];
	shaderEntry.ref--;
	if (shaderEntry.ref == 0)
	{
		printf("DELETING %ls", pShaderName);
	}
}

ShaderList ShaderManager::mShaderList;
