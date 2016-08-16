#pragma once

// Show error function, will get the error message from the result
// Parentwidget is optional parameter
void ShowError(HRESULT pResult, const char* pCalledFunction, QWidget* pParentWidget=nullptr);