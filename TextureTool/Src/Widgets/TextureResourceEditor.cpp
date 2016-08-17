#include "TT_PCH.h"

TextureResourceEditor::TextureResourceEditor(QWidget *parent)
	: QWidget(parent),
	mSharpeningFilter(nullptr),
	mCheckerboardTexture(nullptr),
	mConfigureSharpeningFilterAction(nullptr),
	mImportTextureAction(nullptr),
	mRenderDiffuseAction(nullptr),
	mRenderNormalMapAction(nullptr),
	mRevealInExplorerAction(nullptr),
	mDeleteSelectedTextureAction(nullptr)
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

	// Create the sharpening filter and editor
	mSharpeningFilter = new SharpeningFilter(this);

	mUi.mConfigureSharpeningFilterBtn->setEnabled(false);
	// Enable the customContextMenuRequested signal to be emitted
	mUi.mTextureSelector->setContextMenuPolicy(Qt::CustomContextMenu);

	UpdateRenderPreviewTypeCB();

	connect(mUi.mTextureSelector,              SIGNAL(currentRowChanged(int)),              SLOT(OnTextureSelectorRowChanged()));
	connect(mUi.mTextureSelector,              SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(OnTextureSelectorItemDoubleClicked(QListWidgetItem*)));
	connect(mUi.mTextureSelector,              SIGNAL(customContextMenuRequested(QPoint)),  SLOT(OnTextureSelectorContextMenuRequested()));
	connect(mUi.mCompressionTypeCB,            SIGNAL(currentIndexChanged(int)),            SLOT(OnCompressionTypeCBIndexChanged()));
	connect(mUi.mConvertBtn,                   SIGNAL(pressed()),                           SLOT(OnConvertBtnPressed()));
	connect(mUi.mPreviewMipmapCB,              SIGNAL(currentIndexChanged(int)),            SLOT(OnPreviewMipmapCBIndexChanged()));
	connect(mUi.mRenderPreviewChannelsCB,      SIGNAL(currentIndexChanged(int)),            SLOT(OnRenderPreviewTypeCBIndexChanged()));
	connect(mUi.mTexturePreviewRV,             SIGNAL(Resized()),                           SLOT(OnTexturePreviewRVResized()));
	connect(mUi.mTexturePreviewRV,             SIGNAL(Resized()),                           SLOT(OnTexturePreviewRVResized()));
	connect(mUi.mConfigureSharpeningFilterBtn, SIGNAL(pressed()),                           SLOT(OnConfigureSharpeningFilterBtnPressed()));

	connect(mUi.mUseSharpeningFilterCheckbox,  SIGNAL(stateChanged(int)),                   SLOT(OnUseSharpeningFilterCheckboxChanged()));
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


	QMenuBar* menuBar = mMainFrm->GetMenuBar();

	// Add a sub menu called "Texture" in the top menubar
	QMenu* textureMenu = menuBar->addMenu("Texture");

	mRevealInExplorerAction = new QAction(this);
	mRevealInExplorerAction->setText("Reveil current texture in explorer");
	mRevealInExplorerAction->setIcon(QIcon(":/TextureToolRendering/ReveilInExplorerIcon"));

	textureMenu->addAction(mRevealInExplorerAction);

	mDeleteSelectedTextureAction = new QAction(this);
	mDeleteSelectedTextureAction->setText("Deletes the currently selected texture");
	mDeleteSelectedTextureAction->setIcon(QIcon(":/TextureToolRendering/TrashIcon"));
	mDeleteSelectedTextureAction->setShortcut(QKeySequence::Delete);

	textureMenu->addAction(mDeleteSelectedTextureAction);


	QMenu* configMenu = menuBar->addMenu("Config");

	// Add option to configure the sharpening
	mConfigureSharpeningFilterAction = new QAction(this);
	mConfigureSharpeningFilterAction->setText("Configure Sharpening Filter");
	mConfigureSharpeningFilterAction->setIcon(QIcon(":/TextureToolRendering/SharpeningIcon"));
	mConfigureSharpeningFilterAction->setCheckable(true);
	configMenu->addAction(mConfigureSharpeningFilterAction);

	connect(mConfigureSharpeningFilterAction, SIGNAL(triggered()), SLOT(SharpeningFilterActionTriggered()));


	// Add a sub menu called "Rendering" in the top menubar
	QMenu* renderingMenu = menuBar->addMenu("Rendering");



	// Add option to switch to the diffuse render shader
	mRenderDiffuseAction = new QAction(this);
	mRenderDiffuseAction->setText("Render with diffuse shader");
	mRenderDiffuseAction->setIcon(QIcon(":/TextureToolRendering/DiffuseIcon"));
	mRenderDiffuseAction->setCheckable(true);
	renderingMenu->addAction(mRenderDiffuseAction);


	// Add option to switch to the normalmap render shader
	mRenderNormalMapAction = new QAction(this);
	mRenderNormalMapAction->setText("Render with normal map shader");
	mRenderNormalMapAction->setIcon(QIcon(":/TextureToolRendering/NormalMapIcon"));
	mRenderNormalMapAction->setCheckable(true);


	QToolBar* toolBar = mMainFrm->GetToolBar();


	renderingMenu->addAction(mRenderNormalMapAction);

	mImportTextureAction = mMainFrm->GetImportTextureAction();

	toolBar->addAction(mImportTextureAction);

	toolBar->addSeparator();

	
	// Create a spacer that is a widget, since you can't add QSpacerItems to a toolbar
	QWidget* renderModeSpacerWidget = new QWidget(this);
	renderModeSpacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	toolBar->addWidget(renderModeSpacerWidget);
	mRenderModeSpacerAction = toolBar->actions()[(toolBar->actions().count() - 1)];
	toolBar->addWidget(new QLabel("Render shader: ", this));

	toolBar->addAction(mRenderDiffuseAction);

	toolBar->addAction(mRenderNormalMapAction);

	UpdateRenderingMenuAndToolbar();

	// Connect the actions
	connect(mRevealInExplorerAction, SIGNAL(triggered()), SLOT(RevealInExplorerActionTriggered()));
	connect(mDeleteSelectedTextureAction, SIGNAL(triggered()), SLOT(DeleteSelectedTextureActionTriggered()));

	connect(mRenderDiffuseAction, SIGNAL(triggered()), SLOT(RenderDiffuseActionTriggered()));
	connect(mRenderNormalMapAction, SIGNAL(triggered()), SLOT(RenderNormalMapActionTriggered()));
	connect(mImportTextureAction, SIGNAL(triggered()), SLOT(ImportTextureActionTriggered()));
}

void TextureResourceEditor::UpdateTextureSelector()
{
	const int rowBuffer = mUi.mTextureSelector->currentRow();
	// Make sure signals don't get trigged while clearing/adding items
	mUi.mTextureSelector->blockSignals(true);
	mUi.mTextureSelector->clear();

	for (int i = 0; i < mTextureList.size(); i++)
	{
		QListWidgetItem* item = new QListWidgetItem();

		// Get the filename from the original texture, converted texture might be nullptr
		Texture* texture = mTextureList[i]->originalTexture;
		QString textureFileName = texture->GetFileName();
		item->setText(QTToBaseName(textureFileName.toLatin1().data()));

		// Set the data of this item to the TextureEntry pointer
		QVariant data;
		data.setValue<void*>(mTextureList[i]);
		item->setData(Qt::UserRole, data);

		mUi.mTextureSelector->addItem(item);
	}
	mUi.mTextureSelector->blockSignals(false);
	// Set the row back to where it was when it called the function
	if (rowBuffer != -1)
	{
		mUi.mTextureSelector->setCurrentRow(rowBuffer);
	}
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

void TextureResourceEditor::RevealCurrentTextureInExplorer()
{
	QListWidgetItem* currentItem = mUi.mTextureSelector->currentItem();

	// Don't do anything if nothing is selected
	if (currentItem == nullptr)
		return;

	TextureEntry* textureEntry =(TextureEntry*)currentItem->data(Qt::UserRole).value<void*>();

	// If there's an item, there has to be an textureEntry attached to it
	assert(textureEntry != nullptr);
	Texture* texture = textureEntry->originalTexture;
	QTExplorerOpenFileFolder(texture->GetFileName().toLatin1().data());
}

void TextureResourceEditor::SharpeningFilterActionTriggered()
{
	mSharpeningFilter->Configure();
}

void TextureResourceEditor::RevealInExplorerActionTriggered()
{
	RevealCurrentTextureInExplorer();
}

void TextureResourceEditor::DeleteSelectedTextureActionTriggered()
{
	const int index = mUi.mTextureSelector->currentRow();
	// If the currentrow is -1, it means that nothing is selected, so return
	if (index == -1)
	{
		mMainFrm->SetStatusBar("None selected");
		return;
	}
	TextureEntry* textureEntry = mTextureList[index];
	int DEBUG_Before = mTextureList.size();
	mTextureList.erase(mTextureList.begin() + index, mTextureList.begin() + index+1);
	int DEBUG_After = mTextureList.size();
	delete textureEntry;
	UpdateTextureSelector();
}

void TextureResourceEditor::RenderDiffuseActionTriggered()
{
	mUi.mTexturePreviewRV->SetCurrentPixelShader(PSTEXTUREPREVIEWSHADERNAME_DIFFUSE);
	UpdateRenderingMenuAndToolbar();
	RenderPreviewImage();
}

void TextureResourceEditor::RenderNormalMapActionTriggered()
{
	mUi.mTexturePreviewRV->SetCurrentPixelShader(PSTEXTUREPREVIEWSHADERNAME_NORMALMAP);
	UpdateRenderingMenuAndToolbar();
	RenderPreviewImage();
}

void TextureResourceEditor::ImportTextureActionTriggered()
{
	// Add every image file to be opened under "Any image file"
	QString filter = "Any image file (";
	QByteArrayList supportedImageFormats= QImageReader::supportedImageFormats();
	for (int i = 0; i < supportedImageFormats.count(); i++)
	{
		filter += "*." + supportedImageFormats[i]+ " ";
	}
	filter += ")";

	// Also add all the supported images to be opened seperately
	for (int i = 0; i < supportedImageFormats.count(); i++)
	{
		filter += "*." + supportedImageFormats[i];
		 // If its the last iteration, don't add the seperator
		if (i != supportedImageFormats.count() - 1)
		{
			filter += ";;";
		}
	}
	QStringList imageFiles = QFileDialog::getOpenFileNames(this, "Import Textures", "", filter);
	for (int i = 0; i < imageFiles.count(); i++)
	{
		// Load and add the textures to the list 
		Texture* texture = nullptr;
		HRESULT result = LoadTexture(imageFiles[i].toLatin1().data(), texture);
		if FAILED(result)
		{
			if (result == E_READINGFILEFAILED)
			{
				QMessageBox msgBox;
				msgBox.critical(this, "Loading file failed", "Error loading image");
			}
			else if (result != E_UNSUPPORTEDFILE) // Don't report unsupported file errors 
			{
				ShowError(result, "ImportTexture");
			}
			continue;
		}
		AddTexture(texture);

	}
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
	const bool textureIsPowerOfTwo = QTIsPowerOfTwo(texture->GetFirstSurface());
	SetCBCompressionTypesEnabled(textureIsPowerOfTwo);
	SetSupportedTextureCBToFormat(texture->GetFormat());
	UpdateMipmapCB();
	if (!QTIsPowerOfTwo(texture->GetFirstSurface()))
	{
		mMainFrm->SetStatusBar("Current texture is not power of two so it can not use block compression");
	}
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

void TextureResourceEditor::OnTexturePreviewRVResized()
{
	RenderPreviewImage();
}

void TextureResourceEditor::OnConfigureSharpeningFilterBtnPressed()
{
	mSharpeningFilter->Configure();
}

void TextureResourceEditor::OnUseSharpeningFilterCheckboxChanged()
{
	bool checked = mUi.mUseSharpeningFilterCheckbox->isChecked();
	mUi.mConfigureSharpeningFilterBtn->setEnabled(checked);
	mSharpeningFilter->SetSharpeningEnabled(checked);
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
	int channels = mUi.mRenderPreviewChannelsCB->currentData().value<int>();
	mUi.mTexturePreviewRV->SetRenderChannels(channels);
	RenderPreviewImage();
}

void TextureResourceEditor::OnTextureSelectorItemDoubleClicked(QListWidgetItem* pItem)
{
	mRevealInExplorerAction->trigger();
}


void TextureResourceEditor::OnTextureSelectorContextMenuRequested()
{
	QMenu menu(this);
	menu.addAction(mRevealInExplorerAction);
	menu.addAction(mDeleteSelectedTextureAction);
	menu.exec(QCursor::pos());
}

void TextureResourceEditor::OnCompressionTypeCBIndexChanged()
{
	// If the currentrow is -1, it means that nothing is selected, so return
	if (mUi.mTextureSelector->currentRow() == -1)
	{
		return;
	}
	TextureEntry* textureEntry = mTextureList[mUi.mTextureSelector->currentRow()];

	DXGI_FORMAT format = mUi.mCompressionTypeCB->currentData().value<DXGI_FORMAT>();
	if (format == DXGI_FORMAT_BC5_UNORM || DXGI_FORMAT_BC5_SNORM)
	{
		mMainFrm->SetStatusBar("Use the NormalMap shader for this compression type");
	}
}

void TextureResourceEditor::UpdateRenderingMenuAndToolbar()
{
	const Shader* currentPixelShader = mTexturePreviewRV->GetCurrentPixelShader();

	mRenderDiffuseAction->setChecked(currentPixelShader->GetShaderName().compare(PSTEXTUREPREVIEWSHADERNAME_DIFFUSE) == 0);

	mRenderNormalMapAction->setChecked(currentPixelShader->GetShaderName().compare(PSTEXTUREPREVIEWSHADERNAME_NORMALMAP) == 0);
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
		// Load and add the textures to the list 
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
	mUi.mRenderPreviewChannelsCB->addItem("RGBA(Blended)", QVariant::fromValue<int>(0xFFFFFFFF));
	mUi.mRenderPreviewChannelsCB->addItem("RGB",		   QVariant::fromValue<int>(0xFFFFFF00));

	mUi.mRenderPreviewChannelsCB->addItem("Red",		   QVariant::fromValue<int>(0xFF000000));
	mUi.mRenderPreviewChannelsCB->addItem("Green",		   QVariant::fromValue<int>(0x00FF0000));
	mUi.mRenderPreviewChannelsCB->addItem("Blue",		   QVariant::fromValue<int>(0x0000FF00));
	mUi.mRenderPreviewChannelsCB->addItem("Alpha",		   QVariant::fromValue<int>(0x000000FF));
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
	converter.SetSharpeningFilter(mSharpeningFilter);
	converter.SetGenerateMipmaps(mUi.mGenerateMipmapsCheckbox->isChecked());
	converter.SetDstFormat(pFormat);
	Texture* convertedTexture = nullptr;
	HRESULT result = converter.Process(convertedTexture);
	if FAILED(result)
	{
		ShowError(result, "converter.Convert(convertedTexture)");
		mMainFrm->SetStatusBar("Conversion failed");
	}
	else
	{
		mMainFrm->SetStatusBar("Conversion succeeded");
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

void TextureResourceEditor::SetCBCompressionTypesEnabled(bool pEnabled)
{
	const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(mUi.mCompressionTypeCB->model());

	for (int i = 0; i < mUi.mCompressionTypeCB->count(); i++)
	{
		QVariant variant = mUi.mCompressionTypeCB->itemData(i);
		DXGI_FORMAT format = variant.value<DXGI_FORMAT>();
		// Get the index of the value to disable
		QStandardItem* item = model->item(i);

		// If the format is block compressed, and the Block compression -
		// -types need to be disabled (pEnabled ==true), disable the item
		if(!pEnabled&&!DXIsBlockCompressed(format) || pEnabled)
		{
			item->setToolTip("");
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		}
		else
		{
			item->setToolTip("Texture is not block compressed");
			item->setFlags(~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
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
	if (previewTexture == nullptr)
	{
		previewTexture = textureEntry->originalTexture;
	}
	assert(previewTexture != nullptr); // Converted texture may be null, but the original may not, since we checked for non selection
	return previewTexture;
}

void TextureResourceEditor::RenderPreviewImage()
{
	// Clear the screen with the alpha checkerboard texture,
	// Make sure we render it with all the channels, 
	// Copying the current channels in this buffer
	int renderChannelsBuffer = mTexturePreviewRV->GetRenderChannels();

	mTexturePreviewRV->SetRenderChannels(0xFFFFFFFF);
	mTexturePreviewRV->Render2DTexture(mCheckerboardTexture);
	mTexturePreviewRV->SetRenderChannels(renderChannelsBuffer);

	Texture* previewTexture = GetPreviewingTexture();
	if (previewTexture == nullptr) // If theres no active texture, stop here
	{
		// Still blit the cleared/checkerboarded image 
		mTexturePreviewRV->Blit();
		return;
	}

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

// TextureEntry implementation
TextureEntry::~TextureEntry()
{
	SAFE_DELETE(originalTexture);
	SAFE_DELETE(convertedTexture);
}
