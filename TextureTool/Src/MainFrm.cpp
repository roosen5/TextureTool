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

	mToggleNightModeAction = new QAction(this);
	mToggleNightModeAction->setText("Set to night mode");
	mToggleNightModeAction->setIcon(QIcon(":/TextureToolRendering/NightIcon"));

	GetToolBar()->insertAction(mTextureResourceEditor->GetRenderModeSpacer(),mToggleNightModeAction);

	mUi.mRenderViewFrame->layout()->addWidget(mTextureResourceEditor);
	GetToolBar()->setIconSize(QSize(32, 32));

	// connect
	connect(mUi.mExitApplicationAction, SIGNAL(triggered()), SLOT(OnExitApplicationClicked()));
	connect(mToggleNightModeAction, SIGNAL(triggered()), SLOT(OnToggleNightModeActionClicked()));
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

void MainFrm::OnToggleNightModeActionClicked()
{
	ToggleNightMode();
}

void MainFrm::ToggleNightMode()
{
	if (qApp->styleSheet().isEmpty()) // Empty means no stylesheet, means no nightmode
	{
		QFile file(":/TextureToolRendering/QDarkStyleStyleSheet");
		file.open(QFile::ReadOnly);
		QString styleSheet = QLatin1String(file.readAll());
		qApp->setStyleSheet(styleSheet);
		mToggleNightModeAction->setText("Set to day mode");
		mToggleNightModeAction->setIcon(QIcon(":/TextureToolRendering/DayIcon"));
	}
	else
	{
		qApp->setStyleSheet("");
		mToggleNightModeAction->setText("Set to night mode");
		mToggleNightModeAction->setIcon(QIcon(":/TextureToolRendering/NightIcon"));
	}
}

void MainFrm::resizeEvent(QResizeEvent *event)
{
	Base::resizeEvent(event);
	mTextureResourceEditor->RenderPreviewImage();
}

