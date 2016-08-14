#pragma once
FORWARD_DECLARE_CLASS(Shader);

enum ShaderType 
{
	stNone,
	stVertexShader,
	stPixelShader,
	stUnknownShader,
};

struct ShaderEntry 
{
	Shader* shader;
	int ref;
};

typedef std::map<std::wstring, ShaderEntry> ShaderList;

class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();

	// Checks for the shader type by the last two characters 
	static ShaderType           RetrieveShaderType(const wchar_t* pShaderName);

	static HRESULT              CreatePixelShader(const wchar_t* pShaderName, ID3D11PixelShader*& pPixelShader, ID3DBlob*& pPixelShaderBlob);
	static HRESULT              CreateVertexShader(const wchar_t* pShaderName, ID3D11VertexShader*& pVertexShader, ID3DBlob*& pVertexShaderBlob);

	// Loads the shader, creates it if it's not already loaded
	static Shader*              LoadShader(const wchar_t* pShaderName);
	// Unloads the shader, but if the reference count of the shader is zero, it gets released
	static void                 UnloadShader(const wchar_t* pShaderName);
private:
	static ShaderList           mShaderList;
};

