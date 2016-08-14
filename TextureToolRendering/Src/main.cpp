#include "TT_PCH.h"


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainFrm w;
	w.show();
	int returnValue = a.exec();
	DXDevice::ShutDown();
	return returnValue ;
}
