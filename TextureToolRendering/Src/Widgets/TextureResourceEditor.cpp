#include "TT_PCH.h"

TextureResourceEditor::TextureResourceEditor(QWidget *parent)
	: QWidget(parent),
	  mCheckerboardTexture(nullptr)
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

	LoadTexture("checkerboard.png", mCheckerboardTexture);

	mUi.mTextureSelector->setCurrentRow(0);
	RenderPreviewImage();

	UpdateTextureInfo();


	connect(mUi.mTextureSelector,   SIGNAL(currentRowChanged(int)), SLOT(OnTextureSelectorRowChanged()));
	connect(mUi.mTextureSelector,   SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(OnTextureSelectorItemDoubleClicked(QListWidgetItem* )));
	connect(mUi.mCompressionTypeCB, SIGNAL(currentIndexChanged(int)), SLOT(OnCompressionTypeIndexChanged()));
	connect(mUi.mConvertBtn, SIGNAL(pressed()), SLOT(OnTextureSelectorRowChanged()));
}

TextureResourceEditor::~TextureResourceEditor()
{
	for (int i = 0; i < mTextureList.size(); i++)
	{
		delete mTextureList[i];
	}
	mTextureList.clear();
	SAFE_DELETE(mCheckerboardTexture);
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
	 //If the currentrow is -1, it means that nothing is selected, so return
	if (mUi.mTextureSelector->currentRow() == -1)
	{
		return;
	}
	assert(mUi.mTextureSelector->currentRow() < mTextureList.size());
	Texture* texture = GetPreviewingTexture();
	RenderPreviewImage();
	UpdateTextureInfo();
	SetSupportedTextureCBToFormat(texture->GetFormat());
}

void TextureResourceEditor::OnTextureSelectorItemDoubleClicked(QListWidgetItem* pItem)
{
	TextureEntry* textureEntry = (TextureEntry*)pItem->data(Qt::UserRole).value<void*>();
	Texture* texture = textureEntry->originalTexture;
	QTExplorerOpenFileFolder(texture->GetFileName().toLatin1().data());
}

void TextureResourceEditor::OnCompressionTypeIndexChanged()
{
	DXGI_FORMAT cbFormat = mUi.mCompressionTypeCB->currentData().value<DXGI_FORMAT>();
	ConvertTexture(cbFormat);
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

HRESULT TextureResourceEditor::LoadTexture(const char* pFileName, Texture*& pOutTexture)
{
	assert(pOutTexture == nullptr);
	if (!QFileInfo(pFileName).exists())
	{
		return STG_E_FILENOTFOUND;
	}
	if (!QTSupportsImageFormat(pFileName))
	{
		return E_UNSUPPORTEDFILE;
	}
	// Create out-texture
	pOutTexture = new Texture();

	// Load surface
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
	pOutTexture->Set2DTexture(dx2DTexture);
	pOutTexture->SetFileName(pFileName);

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
	pOutTexture->AddSurface(surface);
	pOutTexture->SetResourceView(surfaceResourceView);
	return result;
}

void TextureResourceEditor::SetupSupportedTextureFormats()
{
	AddSupportedTextureFormat(DXGI_FORMAT_R8G8B8A8_UNORM, "RGBA (32 Bit)");
	AddSupportedTextureFormat(DXGI_FORMAT_BC1_UNORM, "DXT1/BC1");
	AddSupportedTextureFormat(DXGI_FORMAT_BC2_UNORM, "DXT3/BC2");
	AddSupportedTextureFormat(DXGI_FORMAT_BC3_UNORM, "DXT5/BC3"); 
	AddSupportedTextureFormat(DXGI_FORMAT_BC5_UNORM, "BC5"); 
}

void TextureResourceEditor::ConvertTexture(DXGI_FORMAT pFormat)
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
	converter.SetGenerateMipmaps(mUi.mGenerateMipmapsCB->isChecked());
	converter.SetDstFormat(pFormat);
	Texture* convertedTexture = nullptr;
	HRESULT result = converter.Process(convertedTexture);
	if FAILED(result)
	{
		ShowError(result, "converter.Convert(convertedTexture)");
	}
	mTextureList[mUi.mTextureSelector->currentRow()]->convertedTexture = convertedTexture;
	RenderPreviewImage();
	UpdateTextureInfo();
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

void TextureResourceEditor::UpdateTextureInfo()
{
	Texture* texture = GetPreviewingTexture();
	if (texture == nullptr)
		return;
	DXGI_FORMAT DEBUG_Format =  texture->GetFirstSurface()->GetFormat();
	QString text = QString("Width: %1, Height: %2\nBytes: %3").arg(texture->GetWidth()).arg(texture->GetHeight()).arg(texture->GetFirstSurface()->GetDepthPitch());
	mUi.mPreviewTextureInfo->setText(text);
}

TextureEntry* TextureResourceEditor::AddTexture(const Texture* pTexture)
{
	TextureEntry* textureEntry = new TextureEntry(); // Mathijs maybe stack
	textureEntry->originalTexture = (Texture*)pTexture;
	textureEntry->convertedTexture = nullptr;
	mTextureList.push_back(textureEntry);
	UpdateTextureSelector();
	return textureEntry;
}

Texture* TextureResourceEditor::GetPreviewingTexture()
{
	// If the currentrow is -1, it means that nothing is selected, so return
	if (mUi.mTextureSelector->currentRow() == -1) 
	{
		return nullptr;
	}
	TextureEntry* textureEntry = mTextureList[mUi.mTextureSelector->currentRow()];
	Texture* previewTexture = textureEntry->convertedTexture;
	if(previewTexture ==nullptr)
		previewTexture = textureEntry->originalTexture;
	assert(previewTexture != nullptr); // Converted texture may be null, but the original may not, since we checked for non selection
	return previewTexture;
}

void TextureResourceEditor::RenderPreviewImage()
{
	Texture* previewTexture = GetPreviewingTexture();
	if (previewTexture == nullptr)
		return;
	// Clear the screen with the alpha checkerboard texture
	mTexturePreviewRV->Render2DTexture(mCheckerboardTexture);
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
	SAFE_DELETE(originalTexture);
	SAFE_DELETE(convertedTexture);
}
