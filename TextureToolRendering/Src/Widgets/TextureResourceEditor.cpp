#include "TT_PCH.h"

TextureResourceEditor::TextureResourceEditor(QWidget *parent)
	: QWidget(parent), 
	  mPreviewTexture (nullptr)
{
	mUi.setupUi(this);

	//Initialize the texture preview resource view
	mTexturePreviewRV = mUi.mTexturePreviewRV;

	HRESULT result = mTexturePreviewRV->Initialize();
	if FAILED(result)
	{
		ShowError(result, "mTexturePreviewRV->InitializeDirectX");
	}

	setAcceptDrops(true);

	connect(mUi.mTextureSelector, SIGNAL(currentRowChanged(int)), SLOT(OnTextureSelectorRowChanged()));
}

TextureResourceEditor::~TextureResourceEditor()
{
	for (int i = 0; i < mTextureList.size(); i++)
	{
		delete mTextureList[i];
	}
	mTextureList.clear();
}

void TextureResourceEditor::UpdateTextureSelector()
{
	mUi.mTextureSelector->blockSignals(true);
	mUi.mTextureSelector->clear();
	for (int i = 0; i < mTextureList.size(); i++)
	{
		Texture* texture = mTextureList[i];
		QString textureFileName = texture->GetFileName();
		mUi.mTextureSelector->addItem(QTToBaseName(textureFileName.toLatin1().data()));
	}
	mUi.mTextureSelector->blockSignals(false);
}

void TextureResourceEditor::OnTextureSelectorRowChanged()
{
	// If the currentrow is -1, it means that nothing is selected, so return
	if (mUi.mTextureSelector->currentRow() == -1)
	{
		return;
	}
	assert(mUi.mTextureSelector->currentRow() < mTextureList.size());
	Texture* texture = mTextureList[mUi.mTextureSelector->currentRow()];
	assert(texture != nullptr);
	SetPreviewTexture(texture);
}

void TextureResourceEditor::dragLeaveEvent(QDragLeaveEvent* pEvent)
{
	Base::dragLeaveEvent(pEvent);
}

void TextureResourceEditor::dropEvent(QDropEvent* pEvent)
{
	Base::dropEvent(pEvent);

	for (int i = 0; i < pEvent->mimeData()->urls().count(); i++)
	{
		Texture* texture=nullptr;
		QUrl Url = pEvent->mimeData()->urls()[i];
		QString path = Url.toLocalFile();
		HRESULT result = LoadTexture(path.toLatin1().data(), texture);
		if FAILED(result)
		{
			if (result != E_UNSUPPORTEDFILE) // Don't report unsupported file errors 
			{
				ShowError(result, "ImportTexture");
			}
			continue;
		}
		AddTexture(texture);
	}
}

void TextureResourceEditor::SetPreviewTexture(Texture* pPreviewTexture)
{
	mPreviewTexture = pPreviewTexture;
	mTexturePreviewRV->Render2DTexture(mPreviewTexture);
	mTexturePreviewRV->Blit();
}

void TextureResourceEditor::dragMoveEvent(QDragMoveEvent* pEvent)
{
	Base::dragMoveEvent(pEvent);
}

HRESULT TextureResourceEditor::LoadTexture(const char* pFileName, Texture*& pTexture)
{
	assert(pTexture == nullptr);
	pTexture = new Texture();
	// Load surface
	QImage image(pFileName);
	if (!QTSupportsImageFormat(pFileName))
	{
		return E_UNSUPPORTEDFILE;
	}

	// Create texture
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = image.width();
	textureDesc.Height = image.height();
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = image.bits();
	initData.SysMemPitch = static_cast<UINT>(image.bytesPerLine());
	initData.SysMemSlicePitch = static_cast<UINT>(image.byteCount());

	
	ID3D11Texture2D* dx2DTexture;
	HRESULT result = DXDevice::GetDevice()->CreateTexture2D(&textureDesc, &initData, &dx2DTexture);
	if FAILED(result)
	{
		return result;
	}
	pTexture->Set2DTexture(dx2DTexture);
	pTexture->SetFileName(pFileName);

	ID3D11ShaderResourceView* surfaceResourceView;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	memset(&shaderResourceViewDesc, 0, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	result = DXDevice::GetDevice()->CreateShaderResourceView(dx2DTexture, &shaderResourceViewDesc, &surfaceResourceView);
	if FAILED(result)
	{
		return result;
	}
	pTexture->SetResourceView(surfaceResourceView);
	return result;
}

void TextureResourceEditor::AddTexture(const Texture* pTexture)
{
	mTextureList.push_back((Texture*)pTexture);
	UpdateTextureSelector();
}

void TextureResourceEditor::dragEnterEvent(QDragEnterEvent* pEvent)
{
	Base::dragEnterEvent(pEvent);

	if (pEvent->mimeData()->hasUrls())
	{
		// Check if a dragged item(s) is supported
		for (int i = 0; i < pEvent->mimeData()->urls().count();i++)
		{
			QUrl url = pEvent->mimeData()->urls()[i];
			QString filePath =url.toLocalFile();
			if (QTSupportsImageFormat(filePath.toLatin1().data()))
			{
				// Accept the drag event, changing the cursor to a +, to show the user he/she can drop it
				pEvent->accept();
			}
		}
	}
}

void TextureResourceEditor::resizeEvent(QResizeEvent* pEvent)
{
	if (mPreviewTexture == nullptr)
		return;
	mTexturePreviewRV->Render2DTexture(mPreviewTexture);
	mTexturePreviewRV->Blit();
}
