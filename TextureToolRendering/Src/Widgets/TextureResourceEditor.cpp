#include "TT_PCH.h"

TextureResourceEditor::TextureResourceEditor(QWidget *parent)
	: QWidget(parent),
	  mCheckerboardTexture(nullptr),
	  mRenderDiffuseAction(nullptr),
	  mRenderNormalMapAction(nullptr)
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

	UpdateRenderPreviewTypeCB();


	connect(mUi.mTextureSelector,		SIGNAL(currentRowChanged(int)),              SLOT(OnTextureSelectorRowChanged()));
	connect(mUi.mTextureSelector,   	SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(OnTextureSelectorItemDoubleClicked(QListWidgetItem* )));
	connect(mUi.mCompressionTypeCB, 	SIGNAL(currentIndexChanged(int)),            SLOT(OnCompressionTypeIndexChanged()));
	connect(mUi.mConvertBtn,        	SIGNAL(pressed()),                           SLOT(OnConvertBtnPressed()));
	connect(mUi.mPreviewMipmapCB,   	SIGNAL(currentIndexChanged(int)),            SLOT(OnPreviewMipmapCBIndexChanged()));
	connect(mUi.mRenderPreviewTypeCB,   SIGNAL(currentIndexChanged(int)),            SLOT(OnRenderPreviewTypeCBIndexChanged()));
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

void TextureResourceEditor::Setup(MainFrm* pMainFrm)
{
	mMainFrm = pMainFrm;

	mRenderDiffuseAction = new QAction(this);
	mRenderDiffuseAction->setText("Render with diffuse shader");
	mRenderDiffuseAction->setIcon(QIcon(":/TextureToolRendering/DiffuseIcon"));
	mRenderDiffuseAction->setCheckable(true);

	QMenuBar* menuBar = mMainFrm->GetMenuBar();
	mRenderingMenu = menuBar->addMenu("Rendering");


	mRenderNormalMapAction = new QAction(this);
	mRenderNormalMapAction->setText("Render with normal map shader");
	mRenderNormalMapAction->setIcon(QIcon(":/TextureToolRendering/NormalMapIcon"));
	mRenderNormalMapAction->setCheckable(true);


	QToolBar* toolBar = mMainFrm->GetToolBar();

	connect(mRenderDiffuseAction,       SIGNAL(triggered()), SLOT(RenderDiffuseActionTriggered()));
	connect(mRenderNormalMapAction,     SIGNAL(triggered()), SLOT(RenderNormalMapActionTriggered()));

	UpdateRenderingMenu();
}

void TextureResourceEditor::UpdateTextureSelector()
{
	// Make sure signals don't get trigged while clearing/adding items
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

void TextureResourceEditor::UpdateMipmapCB()
{
	// Make sure signals don't get trigged while clearing/adding items
	mUi.mPreviewMipmapCB->blockSignals(true);

	// To go back to that index after refreshing
	int indexBuffer = mUi.mPreviewMipmapCB->currentIndex();
	if (indexBuffer < 0)
		indexBuffer = 0;
	mUi.mPreviewMipmapCB->clear();
	Texture* texture=  GetPreviewingTexture();
	mUi.mPreviewMipmapCB->addItem("(auto)");
	for (int i = 0; i < texture->GetSurfaceCount(); i++)
	{
		QString itemText("mip %1");
		itemText=itemText.arg(i);
		mUi.mPreviewMipmapCB->addItem(itemText);
	}

	// If the index is higher than the amount of mipmaps, clamp it to last mipmap
	indexBuffer = min(indexBuffer, mUi.mPreviewMipmapCB->count() - 1);

	mUi.mPreviewMipmapCB->setCurrentIndex(indexBuffer);
	mUi.mPreviewMipmapCB->blockSignals(false);
}

void TextureResourceEditor::RenderDiffuseActionTriggered()
{
	mUi.mTexturePreviewRV->SetCurrentPixelShader(PSTEXTUREPREVIEWSHADERNAME_DIFFUSE);
	UpdateRenderingMenu();
	RenderPreviewImage();
}

void TextureResourceEditor::RenderNormalMapActionTriggered()
{
	mUi.mTexturePreviewRV->SetCurrentPixelShader(PSTEXTUREPREVIEWSHADERNAME_NORMALMAP);
	UpdateRenderingMenu();
	RenderPreviewImage();
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
	UpdateMipmapCB();
	// Set the mipmap back to (auto)
	mUi.mTexturePreviewRV->SetForcedMipmap(MIPMAP_AUTO);
}

void TextureResourceEditor::OnPreviewMipmapCBIndexChanged()
{
	// (auto) has to be -1, and it's the first entry so subtract one 
	int mipmap = mUi.mPreviewMipmapCB->currentIndex() - 1;
	// If the current index is -1, it means that none is selected, so force (auto)
	if (mUi.mPreviewMipmapCB->currentIndex() == -1)
	{
		mipmap = MIPMAP_AUTO;
	}
	mUi.mTexturePreviewRV->SetForcedMipmap(mipmap);
	RenderPreviewImage();
	UpdateTextureInfo();
}

void TextureResourceEditor::OnConvertBtnPressed()
{
	// If the currentrow is -1, it means that nothing is selected, so return
	if (mUi.mTextureSelector->currentRow() == -1)
	{
		return;
	}
	TextureEntry* textureEntry = mTextureList[mUi.mTextureSelector->currentRow()];

	DXGI_FORMAT cbFormat = mUi.mCompressionTypeCB->currentData().value<DXGI_FORMAT>();
	ConvertTexture(cbFormat, textureEntry);
}

void TextureResourceEditor::OnRenderPreviewTypeCBIndexChanged()
{
	int channels = mUi.mRenderPreviewTypeCB->currentData().value<int>();
	mUi.mTexturePreviewRV->SetRenderChannels(channels);
	RenderPreviewImage();
}

void TextureResourceEditor::OnTextureSelectorItemDoubleClicked(QListWidgetItem* pItem)
{
	TextureEntry* textureEntry = (TextureEntry*)pItem->data(Qt::UserRole).value<void*>();
	Texture* texture = textureEntry->originalTexture;
	QTExplorerOpenFileFolder(texture->GetFileName().toLatin1().data());
}

void TextureResourceEditor::OnCompressionTypeIndexChanged()
{
	// If the currentrow is -1, it means that nothing is selected, so return
	if (mUi.mTextureSelector->currentRow() == -1)
	{
		return;
	}
	TextureEntry* textureEntry = mTextureList[mUi.mTextureSelector->currentRow()];

	DXGI_FORMAT cbFormat = mUi.mCompressionTypeCB->currentData().value<DXGI_FORMAT>();
	ConvertTexture(cbFormat, textureEntry);
}

void TextureResourceEditor::UpdateRenderingMenu()
{
	mRenderingMenu->clear();
	const Shader* currentPixelShader = mTexturePreviewRV->GetCurrentPixelShader();

	mRenderDiffuseAction->setChecked(currentPixelShader->GetShaderName().compare(PSTEXTUREPREVIEWSHADERNAME_DIFFUSE) == 0);
	mRenderingMenu->addAction(mRenderDiffuseAction);

	mRenderNormalMapAction->setChecked(currentPixelShader->GetShaderName().compare(PSTEXTUREPREVIEWSHADERNAME_NORMALMAP) == 0);
	mRenderingMenu->addAction(mRenderNormalMapAction);

	mMainFrm->GetToolBar()->addAction(mRenderDiffuseAction);

	mMainFrm->GetToolBar()->addAction(mRenderNormalMapAction);
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
			if (result == E_READINGFILEFAILED)
			{
				QMessageBox msgBox;
				msgBox.critical(this, "Loading file failed", "Error loading image");
			}
			else
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
			// Make sure signals don't get trigged selecting the row
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
	if (!surface->IsValid())
	{
		return E_READINGFILEFAILED;
	}

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

	// Create the subresourceview
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

void TextureResourceEditor::UpdateRenderPreviewTypeCB()
{
	mUi.mRenderPreviewTypeCB->addItem("RGBA(Blended)", QVariant::fromValue<int>(0xFFFFFFFF));
	mUi.mRenderPreviewTypeCB->addItem("RGB",		   QVariant::fromValue<int>(0xFFFFFF00));

	mUi.mRenderPreviewTypeCB->addItem("Red",		   QVariant::fromValue<int>(0xFF000000));
	mUi.mRenderPreviewTypeCB->addItem("Green",		   QVariant::fromValue<int>(0x00FF0000));
	mUi.mRenderPreviewTypeCB->addItem("Blue",		   QVariant::fromValue<int>(0x0000FF00));
	mUi.mRenderPreviewTypeCB->addItem("Alpha",		   QVariant::fromValue<int>(0x000000FF));

	mUi.mRenderPreviewTypeCB->addItem("Normal map",	   QVariant::fromValue<int>(0x000000FF));
}

void TextureResourceEditor::SetupSupportedTextureFormats()
{
	AddSupportedTextureFormat(DXGI_FORMAT_R8G8B8A8_UNORM, "RGBA (32 Bit)");
	AddSupportedTextureFormat(DXGI_FORMAT_BC1_UNORM,	  "DXT1/BC1");
	AddSupportedTextureFormat(DXGI_FORMAT_BC2_UNORM, 	  "DXT3/BC2");
	AddSupportedTextureFormat(DXGI_FORMAT_BC3_UNORM, 	  "DXT5/BC3"); 
	AddSupportedTextureFormat(DXGI_FORMAT_BC5_UNORM, 	  "BC5"); 
#ifdef ENABLEBC7 // Disabled by default because this compression method takes extremely long
	AddSupportedTextureFormat(DXGI_FORMAT_BC7_UNORM, 	  "BC7(Slow)"); 
#endif
}

void TextureResourceEditor::ConvertTexture(DXGI_FORMAT pFormat, TextureEntry* pTextureEntry)
{
	SAFE_DELETE(pTextureEntry->convertedTexture);
	Texture* originalTexture = pTextureEntry->originalTexture;
	assert(originalTexture != nullptr);
	TextureConverter converter;
	converter.SetOriginalTexture(originalTexture);
	converter.SetGenerateMipmaps(mUi.mGenerateMipmapsCheckbox->isChecked());
	converter.SetDstFormat(pFormat);
	Texture* convertedTexture = nullptr;
	HRESULT result = converter.Process(convertedTexture);
	if FAILED(result)
	{
		ShowError(result, "converter.Convert(convertedTexture)");
	}
	pTextureEntry->convertedTexture = convertedTexture;
	RenderPreviewImage();
	UpdateTextureInfo();
	UpdateMipmapCB();
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
			// Make sure signals don't get trigged selecting the row
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
	int forceMipmap = mUi.mTexturePreviewRV->GetForcedMipmap();
	// If the forceMipmap is set to -1 (AKA MIPMAP_AUTO), set it to zero
	forceMipmap = min(max(0, forceMipmap), texture->GetSurfaceCount() - 1);

	QString text = QString("Width: %1, Height: %2\nMipsize: %3\nMipmap count: %4")
																				.arg(texture->GetSurface(forceMipmap)->GetWidth())
																				.arg(texture->GetSurface(forceMipmap)->GetHeight())
																				.arg(texture->GetSurface(forceMipmap)->GetDepthPitch())
																				.arg(texture->GetSurfaceCount());
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

	// Clear the screen with the alpha checkerboard texture,
	// Make sure we render it with all the channels, 
	// Copying the current channels in this buffer
	int renderChannelsBuffer = mTexturePreviewRV->GetRenderChannels();

	mTexturePreviewRV->SetRenderChannels(0xFFFFFFFF);
	mTexturePreviewRV->Render2DTexture(mCheckerboardTexture);
	mTexturePreviewRV->SetRenderChannels(renderChannelsBuffer);

	// Render the preview texture
	mTexturePreviewRV->Render2DTexture(previewTexture);

	// Blit result to the screen
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
