#include "TT_PCH.h"

MainFrm::MainFrm(QWidget* pParent)
	: QMainWindow(pParent)
{
	// Setup the ui
	mUi.setupUi(this);

	// Create the device
	HRESULT result = DXDevice::InitializeDevice();
	if FAILED(result)
	{
		ShowError(result, "DXDevice::InitializeDevice", this);
		QApplication::quit();
	}

	mUi.mRenderViewFrame->setLayout(new QVBoxLayout(mUi.mRenderViewFrame));
	mRenderInterface = new RenderView();
	mUi.mRenderViewFrame->layout()->addWidget(mRenderInterface );


	result = mRenderInterface->InitializeDirectX();
	if FAILED(result)
	{
		ShowError(result, "renderInterface->InitializeDirectX", this);
		QApplication::quit();
	}

	connect(mUi.mClearBtn, SIGNAL(clicked()), SLOT(OnClearBtnClicked()));
	connect(mUi.mRenderPlaneBtn, SIGNAL(clicked()), SLOT(OnRenderPlaneBtnClicked()));
	connect(mUi.mBlitBtn, SIGNAL(clicked()), SLOT(OnBlitBtnClicked()));
}

MainFrm::~MainFrm()
{
	delete mRenderInterface;
}

void MainFrm::Update()
{
	mRenderInterface->Render2DTexture();
	mRenderInterface->Blit();
}

void MainFrm::OnClearBtnClicked()
{
	mRenderInterface->Clear(QColor(Qt::red));
}

void MainFrm::OnRenderPlaneBtnClicked()
{
	mRenderInterface->Render2DTexture();
}

void MainFrm::OnBlitBtnClicked()
{
	mRenderInterface->Blit();
}

void MainFrm::resizeEvent(QResizeEvent *event)
{
	Base::resizeEvent(event);
}
