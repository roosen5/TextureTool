#include "TT_PCH.h"


int main(int argc, char *argv[])
{
	QApplication application(argc, argv);
	// The entire application is run through the MainFrm, keep the main.cpp as clean as possible
	MainFrm mainFrm;
	mainFrm.show();
	mainFrm.GetTextureResourceEditor()->RenderPreviewImage();
	int returnValue = application.exec();
	// Application ended, shutdown DirectX
	DXDevice::ShutDown();
	return returnValue ;
}
