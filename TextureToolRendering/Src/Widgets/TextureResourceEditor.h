#ifndef TEXTURERESOURCEEDITOR_H
#define TEXTURERESOURCEEDITOR_H

#include <QWidget>
#include "ui_TextureResourceEditor.h"

FORWARD_DECLARE_CLASS(Texture)
FORWARD_DECLARE_CLASS(RenderView)
FORWARD_DECLARE_CLASS(RenderInterface)

Q_DECLARE_METATYPE(DXGI_FORMAT)

struct TextureEntry // MATHIJS maybe class
{
	TextureEntry():originalTexture(nullptr), convertedTexture(nullptr){} 
	~TextureEntry();
	// The texture that will not be changed, 
	Texture* originalTexture; 
	Texture* convertedTexture;
};


class TextureResourceEditor : public QWidget
{
	Q_OBJECT

	typedef QWidget Base;

public:
	TextureResourceEditor(QWidget *parent = 0);
	~TextureResourceEditor();

protected:
	void                         SetPreviewTexture(Texture* pPreviewTexture);

	// Syncs the selector to the resources
	void                         UpdateTextureSelector();

	// Loads the texture, doesn't add it to the resource list
	HRESULT                      LoadTexture(const char* pFileName, Texture*& pTexture);

	// Create supported formats list
	void                         SetupSupportedTextureFormats();

	// All the conversion functions happen inside
	void                         ConvertTexture();

	// Create supported formats list
	void                         AddSupportedTextureFormat(DXGI_FORMAT pFormat, const QString& pFormatName);
	
	// Iterates through the supported texture list to find the format
	void                         SetSupportedTextureCBToFormat(DXGI_FORMAT pFormat);

	// Adds the texture to the resource list
	void                         AddTexture(const Texture* pTexture);

	void                         RenderPreviewImage();
	// Drag and drop functions
	virtual void                 dragEnterEvent(QDragEnterEvent *event) override;

	virtual void                 dragMoveEvent(QDragMoveEvent *event) override;

	virtual void                 dragLeaveEvent(QDragLeaveEvent *event) override;

	virtual void                 dropEvent(QDropEvent *event) override;

	// Catch resize events
	virtual void                 resizeEvent(QResizeEvent *event) override;

private slots:
	void                         OnTextureSelectorRowChanged();
	void                         OnCompressionTypeIndexChanged();
private:


	// The list of textures
	std::vector<TextureEntry*>   mTextureList;

	// The RenderView where the texture will be previewed on
	RenderView*                  mTexturePreviewRV;

	// The RenderInterface used to preview images 
	RenderInterface*             mTexturePreviewRI; 

	Ui::TextureResourceEditor    mUi;

};

#endif // TEXTURERESOURCEEDITOR_H
