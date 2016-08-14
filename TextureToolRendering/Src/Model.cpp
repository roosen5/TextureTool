#include "TT_PCH.h"
#include "Model.h"


Model::Model()
{
	mMaterial.SetPixelShader(ShaderManager::LoadShader(L"TexturePreviewPS"));
	mMaterial.SetVertexShader(ShaderManager::LoadShader(L"TexturePreviewVS"));
	mMaterial.SetupInputLayout();
	LoadPlaneVertices();
}


Model::~Model()
{
}

void Model::Render()
{
}


HRESULT Model::LoadPlaneVertices()
{
	// create a triangle using the VERTEX struct
	Vertex vertices[6];
	vertices[0].Position = DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f);
	vertices[0].TexCoord = DirectX::XMFLOAT2(1, 1);

	vertices[1].Position = DirectX::XMFLOAT3(1.0, 1.0f, 0.0f);
	vertices[1].TexCoord = DirectX::XMFLOAT2(1, 1);

	vertices[2].Position = DirectX::XMFLOAT3(1.0, -1.0, 0.0f);
	vertices[2].TexCoord = DirectX::XMFLOAT2(0, 1);



	vertices[3].Position = DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f);
	vertices[3].TexCoord = DirectX::XMFLOAT2(1, 1);

	vertices[4].Position = DirectX::XMFLOAT3(1.0, -1.0, 0.0f);
	vertices[4].TexCoord = DirectX::XMFLOAT2(0, 1);

	vertices[5].Position = DirectX::XMFLOAT3(-1.0, -1.0f, 0.0f);
	vertices[5].TexCoord = DirectX::XMFLOAT2(1, 1);

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

void Material::SetupInputLayout()
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
