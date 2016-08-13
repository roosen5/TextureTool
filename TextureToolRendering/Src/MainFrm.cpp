#include "TT_PCH.h"

MainFrm::MainFrm(QWidget* pParent)
	: QMainWindow(pParent)
{
	mUi.setupUi(this);
	HRESULT result = DXDevice::InitializeDevice();
	if FAILED(result)
	{
		ShowError(result, "DXDevice::InitializeDevice", this);
		QApplication::quit();
	}


	mUi.mRenderViewFrame->setLayout(new QVBoxLayout(mUi.mRenderViewFrame));
	RenderView* renderView = new RenderView(this);
	mUi.mRenderViewFrame->layout()->addWidget(renderView);

	RenderInterface* renderInterface = new RenderInterface(renderView);

	result = renderInterface->InitializeDirectX();
	if FAILED(result)
	{
		ShowError(result, "renderInterface->InitializeDirectX", this);
		QApplication::quit();
	}
	renderInterface->Clear(QColor(Qt::green));
}

MainFrm::~MainFrm()
{

}
