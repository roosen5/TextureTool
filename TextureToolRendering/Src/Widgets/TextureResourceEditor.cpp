#include "TT_PCH.h"

TextureResourceEditor::TextureResourceEditor(QWidget *parent)
	: QWidget(parent)
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

	SetupSupportedTextureFormats();

	//Texture* texture = nullptr;
	//LoadTexture("C:/GIT/TextureTool/TextureTool/Wood.jpg", texture);
	//AddTexture(texture);
	
	connect(mUi.mTextureSelector, SIGNAL(currentRowChanged(int)), SLOT(OnTextureSelectorRowChanged()));
	connect(mUi.mCompressionTypeCB, SIGNAL(currentIndexChanged(int)), SLOT(OnCompressionTypeIndexChanged()));
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
		QListWidgetItem* Item = new QListWidgetItem();
		// Get the filename from the original texture, converted texture might be nullptr
		Texture* texture = mTextureList[i]->originalTexture;
		QString textureFileName = texture->GetFileName();
		Item->setText(QTToBaseName(textureFileName.toLatin1().data()));

		// Set the data of this item to the TextureEntry pointer
		QVariant data;
		data.setValue<void*>(mTextureList[i]);
		Item->setData(Qt::UserRole, data);

		mUi.mTextureSelector->addItem(Item);
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
	Texture* texture = mTextureList[mUi.mTextureSelector->currentRow()]->convertedTexture;
	if(texture ==nullptr) // Not converted yet, render the original
		texture = mTextureList[mUi.mTextureSelector->currentRow()]->originalTexture;
	assert(texture != nullptr); // Can't be null
	RenderPreviewImage();
}

void TextureResourceEditor::OnCompressionTypeIndexChanged()
{
	ConvertTexture();
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
		Texture* texture = nullptr;
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
	if (pPreviewTexture == nullptr)
		return;
	SetSupportedTextureCBToFormat(pPreviewTexture->GetFormat());
	for (int i = 0; i < mUi.mTextureSelector->count(); i++)
	{
		QListWidgetItem* item = mUi.mTextureSelector->item(i);
		TextureEntry* textureEntry = (TextureEntry*)item->data(Qt::UserRole).value<void*>();
		// Look through all the items in the list and set it to that index if it matches the parameter texture
		if (textureEntry->originalTexture == pPreviewTexture || textureEntry->convertedTexture == pPreviewTexture)
		{
			mUi.mTextureSelector->blockSignals(true);
			mUi.mTextureSelector->setCurrentRow(i);
			mUi.mTextureSelector->blockSignals(false);
		}
	}
	RenderPreviewImage();
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
	if (!QTSupportsImageFormat(pFileName))
	{
		return E_UNSUPPORTEDFILE;
	}
	Surface* surface = new Surface(pFileName);

	// Create texture
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = surface->GetWidth();
	textureDesc.Height = surface->GetHeight();
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem          = surface->GetData();
	initData.SysMemPitch      = static_cast<UINT>(surface->GetRowPitch());
	initData.SysMemSlicePitch = static_cast<UINT>(surface->GetDepthPitch());

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
	pTexture->AddSurface(surface);
	pTexture->SetResourceView(surfaceResourceView);
	return result;
}

void TextureResourceEditor::SetupSupportedTextureFormats()
{
	AddSupportedTextureFormat(DXGI_FORMAT_R8G8B8A8_UNORM, "RGBA (32 Bit)");
	AddSupportedTextureFormat(DXGI_FORMAT_BC1_UNORM, "DXT1/BC1");
	AddSupportedTextureFormat(DXGI_FORMAT_BC3_UNORM, "DXT3/BC2");
	AddSupportedTextureFormat(DXGI_FORMAT_BC5_UNORM, "DXT5/BC3"); 
}

void TextureResourceEditor::ConvertTexture()
{
	// If the currentrow is -1, it means that nothing is selected, so return
	if (mUi.mTextureSelector->currentRow() == -1)
	{
		return;
	}

	SAFE_DELETE(mTextureList[mUi.mTextureSelector->currentRow()]->convertedTexture);
	Texture* originalTexture = mTextureList[mUi.mTextureSelector->currentRow()]->originalTexture;
	assert(originalTexture != nullptr);
	TextureConverter converter;
	converter.SetOriginalTexture(originalTexture);
	Texture* convertedTexture = nullptr;
	HRESULT result = converter.Convert(convertedTexture);
	if FAILED(result)
	{
		ShowError(result, "converter.Convert(convertedTexture)");
	}
	mTextureList[mUi.mTextureSelector->currentRow()]->convertedTexture = convertedTexture;
	RenderPreviewImage();
}

void TextureResourceEditor::AddSupportedTextureFormat(DXGI_FORMAT pFormat, const QString& pFormatName)
{
	QVariant variant;
	variant.setValue<DXGI_FORMAT>(pFormat);
	mUi.mCompressionTypeCB->addItem(pFormatName, variant);
}

void TextureResourceEditor::SetSupportedTextureCBToFormat(DXGI_FORMAT pFormat)
{
	for (int i = 0; i < mUi.mCompressionTypeCB->count(); i++)
	{
		// Combobox format
		DXGI_FORMAT cbFormat = mUi.mCompressionTypeCB->itemData(i).value<DXGI_FORMAT>();
		if (cbFormat == pFormat)
		{
			mUi.mCompressionTypeCB->blockSignals(true);
			mUi.mCompressionTypeCB->setCurrentIndex(i);
			mUi.mCompressionTypeCB->blockSignals(false);
		}
	}
}

void TextureResourceEditor::AddTexture(const Texture* pTexture)
{
	TextureEntry* textureEntry = new TextureEntry(); // Mathijs maybe stack
	textureEntry->originalTexture = (Texture*)pTexture;
	textureEntry->convertedTexture = nullptr;
	mTextureList.push_back(textureEntry);
	UpdateTextureSelector();
}

void TextureResourceEditor::RenderPreviewImage()
{
	// If the currentrow is -1, it means that nothing is selected, so return
	if (mUi.mTextureSelector->currentRow() == -1) 
	{
		return;
	}
	TextureEntry* textureEntry = mTextureList[mUi.mTextureSelector->currentRow()];
	Texture* previewTexture = textureEntry->convertedTexture;
	if(previewTexture ==nullptr)
		previewTexture = textureEntry->originalTexture;
	assert(previewTexture != nullptr); // Converted texture may be null, but the original may not
	mTexturePreviewRV->Render2DTexture(previewTexture);
	mTexturePreviewRV->Blit();
}

void TextureResourceEditor::dragEnterEvent(QDragEnterEvent* pEvent)
{
	Base::dragEnterEvent(pEvent);

	if (pEvent->mimeData()->hasUrls())
	{
		// Check if a dragged item(s) is supported
		for (int i = 0; i < pEvent->mimeData()->urls().count(); i++)
		{
			QUrl url = pEvent->mimeData()->urls()[i];
			QString filePath = url.toLocalFile();
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
	RenderPreviewImage();
}

// TextureEntry implementation
TextureEntry::~TextureEntry()
{
	delete originalTexture;
	delete convertedTexture;
}
