#pragma once

// Checks if the extension is supported
bool    QTSupportsImageFormat(const char* pImageFilePath);

// Extracts the extension
QString QTExtractExtention(const char* pFilePath) ;

// Starts a explorer process and openens the containing folder of the file and selects it
void QTExplorerOpenFileFolder(const char* pFilePath) ;

// Gets the base name
QString QTToBaseName(const char* pFilePath);

DXGI_FORMAT QTFromImageFormat(QImage::Format pFormat);

bool QTIsPowerOfTwo(QImage* pImage);