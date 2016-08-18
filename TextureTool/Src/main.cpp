#include "TT_PCH.h"


int main(int argc, char *argv[])
{
	QApplication application(argc, argv);
	application.setWindowIcon(QIcon(":/TextureToolRendering/MainWindowIcon"));
	// The entire application is run through the MainFrm, keep the main.cpp as clean as possible
	MainFrm mainFrm;
	mainFrm.show();
	int returnValue = application.exec();
	// Application ended, shutdown DirectX
	DXDevice::ShutDown();
	return returnValue ;
}
