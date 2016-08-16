#include "TT_PCH.h"


int main(int argc, char *argv[])
{
	QApplication application(argc, argv);
	MainFrm mainFrm;
	mainFrm.show();
	mainFrm.GetTextureResourceEditor()->RenderPreviewImage();
	int returnValue = application.exec();
	DXDevice::ShutDown();
	return returnValue ;
}
