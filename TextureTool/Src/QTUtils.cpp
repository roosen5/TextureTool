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
	case QImage::Format_RGBA8888:
	case QImage::Format_RGB32:
	case QImage::Format_ARGB32:
	case QImage::Format_ARGB32_Premultiplied:
		return DXGI_FORMAT_R8G8B8A8_UNORM;

	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}

int IsPowerOfTwo(unsigned int x)
{
	return ((x != 0) && !(x & (x - 1)));
}

bool QTIsPowerOfTwo(const Surface* pImage)
{
	if (!IsPowerOfTwo(pImage->GetWidth()))
		return false;
	if (!IsPowerOfTwo(pImage->GetHeight()))
		return false;
	return true;
}
