#include "TT_PCH.h"

void ShowError(HRESULT pResult, const char* pCalledFunction, QWidget* pParentWidget)
{
	_com_error error(pResult);
	QString errorMsg = QString("Function %1 failed\nError:\n\n%2")
		.arg(pCalledFunction, QString::fromWCharArray(error.ErrorMessage()));
	QMessageBox msgBox;
	msgBox.critical(pParentWidget, "Initialization failed", errorMsg);
}