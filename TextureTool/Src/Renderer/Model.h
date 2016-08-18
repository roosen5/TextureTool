#pragma once

FORWARD_DECLARE_CLASS(RenderView);

struct Vertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 TexCoord;
};

struct TexturePreviewInfo
{
	int  mForceMip;
	int  mRenderChannels;
	bool mGrayScaleOutput;
	int  mStride3;
};

class TexturePreviewMaterial
{
public:
	TexturePreviewMaterial();
	~TexturePreviewMaterial();

	// Create the input layout for the vertex shader.
	void                      SetupInputLayout();

	// Sets the vertex shader, when using this function, make sure the current mVertexShader is released
	void                      SetVertexShader(Shader* pVertexShader) { mVertexShader = pVertexShader; }
	// Get the vertex shader
	Shader*                   GetVertexShader(){ return mVertexShader; }
	const Shader*             GetVertexShader() const { return mVertexShader; }

	// Sets the pixel shader, when using this function, make sure the current mPixelShader is released
	void                      SetPixelShader(Shader* pPixelShader) { mPixelShader = pPixelShader; }
	// Get the pixel shader
	Shader*                   GetPixelShader(){ return mPixelShader; }
	const Shader*             GetPixelShader() const { return mPixelShader; }

	// Get and set the inputlayout buffer
	void                      SetInputLayout(ID3D11InputLayout* pLayout) { mInputLayout= pLayout; }
	const ID3D11InputLayout*  GetInputLayout() const { return mInputLayout; }

	// Get and set the previewinfo, to set variables mForceMip and mRenderChannels 
	void					  SetTexturePreviewInfo(const TexturePreviewInfo& pTexturePreviewInfo);
	TexturePreviewInfo		  GetTexturePreviewInfo() { return mTexturePreviewInfo;}
	const TexturePreviewInfo  GetTexturePreviewInfo() const { return mTexturePreviewInfo;}

	// SafeRelease the vertexshader
	void                      ReleaseVertexShader();

	// SafeRelease the pixelshader
	void                      ReleasePixelShader();

	// Creates the previewinfobuffer, should only be called once in a models lifetime
	void					  CreatePreviewInfoBuffer();

	// Only to read, cannot use this function to write, that's why it's constant
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

	// Create a plane mesh 
	HRESULT                        LoadPlaneVertices();

	// Gets the vertexbuffer
	ID3D11Buffer*                  GetVertexBuffer()   { return mVertexBuffer; }

	// Gets the attached material
	TexturePreviewMaterial*		   GetMaterial()	   { return &mMaterial;}
	const TexturePreviewMaterial*  GetMaterial() const { return &mMaterial;}

private:
// The vertex buffer
	ID3D11Buffer*                  mVertexBuffer;
// The material
	TexturePreviewMaterial         mMaterial;
};

