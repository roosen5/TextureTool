#pragma once

// Checks if the extension is supported
bool        QTSupportsImageFormat(const char* pImageFilePath);

// Extracts the extension
QString     QTExtractExtention(const char* pFilePath) ;

// Starts a explorer process and openens the containing folder of the file and selects it
void        QTExplorerOpenFileFolder(const char* pFilePath) ;

// Gets the base name
QString     QTToBaseName(const char* pFilePath);

// Converts the QImage Format to the DirectXFormat
DXGI_FORMAT QTFromImageFormat(QImage::Format pFormat);

// Check if the image is a power of 2 (Required for block compression)
bool        QTIsPowerOfTwo(QImage* pImage);