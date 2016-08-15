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

void QTExplorerOpenFileFolder(const char* pFilePath)
{
	QStringList args;

	args << "/select," << QDir::toNativeSeparators(pFilePath);

	QProcess* process = new QProcess();
	process->start("explorer.exe", args);
}

QString QTToBaseName(const char* pFilePath)
{
	QFileInfo fileInfo(pFilePath);
	return fileInfo.baseName();
};

DXGI_FORMAT QTFromImageFormat(QImage::Format pFormat)
{
	switch (pFormat)
	{
	case QImage::Format_RGB32:
	case QImage::Format_ARGB32:
	case QImage::Format_ARGB32_Premultiplied:
		return DXGI_FORMAT_R8G8B8A8_UNORM;

	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}

bool QTIsPowerOfTwo(QImage* pImage)
{
	const int width = pImage->width();
	if (!(width & (width - 1)) == 0 (width & (width - 1)) == 0)
		return false;
	const int height = pImage->height();
	if (!(height & (height - 1)) == 0 (height & (height - 1)) == 0)
		return false;
	return true;
}
