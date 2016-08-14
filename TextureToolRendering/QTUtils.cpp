#include "TT_PCH.h"

bool QTSupportsImageFormat(const char* pImageFilePath)
{
	QString imageExtension = QTExtractExtention(pImageFilePath);
	QByteArrayList supportedImageFormats = QImageReader::supportedImageFormats();
	for (int i = 0; i < supportedImageFormats.count(); i++)
	{
		if (imageExtension.compare(supportedImageFormats[i], Qt::CaseInsensitive) == 0)
		{
			return true;
		}
	}
	return false;
}

QString QTExtractExtention(const char* pFilePath)
{
	QFileInfo fileInfo(pFilePath);
	return fileInfo.suffix();
}

QString QTToBaseName(const char* pFilePath)
{
	QFileInfo fileInfo(pFilePath);
	return fileInfo.baseName();
};
