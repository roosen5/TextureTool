#pragma once

// Checks if the extension is supported
bool    QTSupportsImageFormat(const char* pImageFilePath);

// Extracts the extension
QString QTExtractExtention(const char* pFilePath) ;

// Gets the base name
QString QTToBaseName(const char* pFilePath);