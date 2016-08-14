#include "TT_PCH.h"

MainFrm::MainFrm(QWidget* pParent)
	: QMainWindow(pParent),
	mRenderInterface(nullptr)
{
	// Setup the ui
	mUi.setupUi(this);

	// Create the device
	HRESULT result = DXDevice::InitializeDevice();
	//DXDevice::ShutDown(); // MEMTEST
	if FAILED(result)
	{
		ShowError(result, "DXDevice::InitializeDevice", this);
		QApplication::quit();
	}

	mUi.mRenderViewFrame->setLayout(new QVBoxLayout(mUi.mRenderViewFrame));
	mRenderInterface = new RenderView();


	result = mRenderInterface->Initialize();
	if FAILED(result)
	{
		ShowError(result, "renderInterface->InitializeDirectX", this);
		QApplication::quit();
	}

	TextureResourceEditor* textureResourceEditor = new TextureResourceEditor(this);
	mUi.mRenderViewFrame->layout()->addWidget(textureResourceEditor);
}

MainFrm::~MainFrm()
{
	SAFE_DELETE(mRenderInterface);
}

void MainFrm::Update()
{
}

void MainFrm::resizeEvent(QResizeEvent *event)
{
	Base::resizeEvent(event);
}
