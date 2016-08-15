#pragma once

FORWARD_DECLARE_CLASS(RenderView);

struct Vertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 TexCoord;
};

struct TexturePreviewInfo
{
	int forceMip;
	int stride1;
	int stride2;
	int stride3;
};

class TexturePreviewMaterial
{
public:
	TexturePreviewMaterial();
	~TexturePreviewMaterial();

	// Create the input layout for the vertex shader.
	void                      SetupInputLayout();

	void                      SetVertexShader(Shader* pVertexShader){ mVertexShader = pVertexShader; }
	Shader*                   GetVertexShader(){ return mVertexShader; }
	const Shader*             GetVertexShader() const { return mVertexShader; }

	void                      SetPixelShader(Shader* pPixelShader){ mPixelShader = pPixelShader; }
	Shader*                   GetPixelShader(){ return mPixelShader; }
	const Shader*             GetPixelShader() const { return mPixelShader; }

	void                      SetInputLayout(ID3D11InputLayout* pLayout) { mInputLayout= pLayout; }
	const ID3D11InputLayout*  GetInputLayout() const { return mInputLayout; }

	void					  SetTexturePreviewInfo(const TexturePreviewInfo& pTexturePreviewInfo);
	TexturePreviewInfo		  GetTexturePreviewInfo() { return mTexturePreviewInfo;}

	void					  CreatePreviewInfoBuffer();

	const ID3D11Buffer*       GetTexturePreviewInfoBuffer() const { return mTexturePreviewInfoBuffer; }

private:
	// This struct will be sent to the shader as constant buffer
	TexturePreviewInfo		  mTexturePreviewInfo;
	// The buffer of the previewinfo
	ID3D11Buffer*             mTexturePreviewInfoBuffer;
	// The vertex shader
	Shader*                   mVertexShader;
	// The pixel shader
	Shader*                   mPixelShader;
	// The input layout buffer
	ID3D11InputLayout*        mInputLayout;
};

class Model
{
public:
	Model();
	~Model();

	void                           Render();

	HRESULT                        LoadPlaneVertices();

	ID3D11Buffer*                  GetVertexBuffer() { return mVertexBuffer; }

	const TexturePreviewMaterial*  GetMaterial() { return &mMaterial;}

private:
// The vertex buffer
	ID3D11Buffer*                  mVertexBuffer;
// The material
	TexturePreviewMaterial         mMaterial;
};

