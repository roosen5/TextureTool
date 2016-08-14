#pragma once
FORWARD_DECLARE_CLASS(Shader);
struct ShaderEntry 
{
	Shader* shader;
	int ref;
};

typedef std::map<std::wstring, ShaderEntry> ShaderList;

enum ShaderType 
{
	stNone,
	stVertexShader,
	stPixelShader,
	stUnknownShader,
};

class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();
	static ShaderType           RetrieveShaderType(const wchar_t* pShaderName);
	static HRESULT              CreatePixelShader(const wchar_t* pShaderName, ID3D11PixelShader*& pPixelShader, ID3DBlob*& pPixelShaderBlob);
	static HRESULT              CreateVertexShader(const wchar_t* pShaderName, ID3D11VertexShader*& pVertexShader, ID3DBlob*& pVertexShaderBlob);
	static Shader*              LoadShader(const wchar_t* pShaderName);
	static void                 UnloadShader(const wchar_t* pShaderName);
private:
	static ShaderList           mShaderList;
};

