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

	mTextureResourceEditor = new TextureResourceEditor(this);
	mTextureResourceEditor->Setup(this);
	mUi.mRenderViewFrame->layout()->addWidget(mTextureResourceEditor);
	GetToolBar()->setIconSize(QSize(32, 32));

	// connect
	connect(mUi.mExitApplicationAction, SIGNAL(triggered()), SLOT(OnExitApplicationClicked()));
}

MainFrm::~MainFrm()
{
}

void MainFrm::SetStatusBar(const QString& pStatus)
{
	mUi.mStatusBar->showMessage(pStatus, 6000);
}

void MainFrm::OnExitApplicationClicked()
{
	QApplication::quit();
}

void MainFrm::resizeEvent(QResizeEvent *event)
{
	Base::resizeEvent(event);
	mTextureResourceEditor->RenderPreviewImage();
}

