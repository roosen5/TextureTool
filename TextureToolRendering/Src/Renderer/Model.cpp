#include "TT_PCH.h"

Model::Model()
{
	// Set the pixel shader default to diffuse
	mMaterial.SetPixelShader(ShaderManager::LoadShader(PSTEXTUREPREVIEWSHADERNAME_DIFFUSE));
	mMaterial.SetVertexShader(ShaderManager::LoadShader(VSTEXTUREPREVIEWSHADERNAME));

	// Create the layout and preview info buffer(Which is the same for every shader)
	mMaterial.SetupInputLayout();
	mMaterial.CreatePreviewInfoBuffer();

	LoadPlaneVertices();
}


Model::~Model()
{
	SAFE_RELEASE(mVertexBuffer);
}


HRESULT Model::LoadPlaneVertices()
{
	// create a triangle using the VERTEX struct
	Vertex vertices[6];
	vertices[0].Position = DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f);
	vertices[0].TexCoord = DirectX::XMFLOAT2(0, 0);

	vertices[1].Position = DirectX::XMFLOAT3(1.0, 1.0f, 0.0f);
	vertices[1].TexCoord = DirectX::XMFLOAT2(1, 0);

	vertices[2].Position = DirectX::XMFLOAT3(1.0, -1.0, 0.0f);
	vertices[2].TexCoord = DirectX::XMFLOAT2(1, 1);

	vertices[3].Position = DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f);
	vertices[3].TexCoord = DirectX::XMFLOAT2(0, 0);

	vertices[4].Position = DirectX::XMFLOAT3(1.0, -1.0, 0.0f);
	vertices[4].TexCoord = DirectX::XMFLOAT2(1, 1);

	vertices[5].Position = DirectX::XMFLOAT3(-1.0, -1.0f, 0.0f);
	vertices[5].TexCoord = DirectX::XMFLOAT2(0, 1);

	// create the vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));

	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;						// write access access by CPU and GPU
	bufferDesc.ByteWidth = sizeof(Vertex) * _countof(vertices); // size is the VERTEX struct * 3
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;			// use as a vertex buffer
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;			// allow CPU to write in buffer

	HRESULT result = DXDevice::GetDevice()->CreateBuffer(&bufferDesc, NULL, &mVertexBuffer);       // create the buffer
	if FAILED(result)
	{
		return result;
	}

	D3D11_MAPPED_SUBRESOURCE subresource;															  // copy the vertices into the buffer
	result = DXDevice::GetContext()->Map(mVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subresource);    // map the buffer
	if FAILED(result)
	{
		return result;
	}
	memcpy(subresource.pData, vertices, sizeof(vertices));											  // copy the data
	DXDevice::GetContext()->Unmap(mVertexBuffer, NULL);
	return result;
}

// TexturePreviewMaterial Implementation

TexturePreviewMaterial::TexturePreviewMaterial():
	mVertexShader(nullptr),
	mPixelShader(nullptr),
	mInputLayout(nullptr)
{

}

TexturePreviewMaterial::~TexturePreviewMaterial()
{
	ReleaseVertexShader();

	ReleasePixelShader();

	SAFE_RELEASE(mInputLayout);
	SAFE_RELEASE(mTexturePreviewInfoBuffer);
}

void TexturePreviewMaterial::SetupInputLayout()
{
	// Create the input layout for the vertex shader.
	D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex,Position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex,TexCoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HRESULT result = DXDevice::GetDevice()->CreateInputLayout(vertexLayoutDesc,
					 _countof(vertexLayoutDesc), GetVertexShader()->GetBlob()->GetBufferPointer(),
					 GetVertexShader()->GetBlob()->GetBufferSize(), &mInputLayout);

	if FAILED(result)
	{
		ShowError(result, "DXDevice::GetDevice()->CreateInputLayout");
	}
}

void TexturePreviewMaterial::SetTexturePreviewInfo(const TexturePreviewInfo& pTexturePreviewInfo)
{
	mTexturePreviewInfo = pTexturePreviewInfo;
	DXDevice::GetContext()->UpdateSubresource(mTexturePreviewInfoBuffer, 0, nullptr, &mTexturePreviewInfo, 0, 0);
}

void TexturePreviewMaterial::ReleaseVertexShader()
{
	if (mVertexShader != nullptr)
	{
		ShaderManager::UnloadShader(mVertexShader->GetShaderName().c_str());
	}
}

void TexturePreviewMaterial::ReleasePixelShader()
{
	if (mPixelShader != nullptr)
	{
		ShaderManager::UnloadShader(mPixelShader->GetShaderName().c_str());
	}
}

void TexturePreviewMaterial::CreatePreviewInfoBuffer()
{
	// Create the constant buffers for the variables defined in the vertex shader.
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof(TexturePreviewInfo);
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	HRESULT result = DXDevice::GetDevice()->CreateBuffer(&constantBufferDesc, nullptr, &mTexturePreviewInfoBuffer);
	if FAILED(result)
	{
		ShowError(result, "DXDevice::GetDevice()->CreateBuffer");
	}
	TexturePreviewInfo previewInfo;
	// Set it to auto by default
	previewInfo.mForceMip = MIPMAP_AUTO;
	SetTexturePreviewInfo(previewInfo);
}

