#ifndef TEXTURERESOURCEEDITOR_H
#define TEXTURERESOURCEEDITOR_H

#include <QWidget>
#include "ui_TextureResourceEditor.h"

FORWARD_DECLARE_CLASS(Texture);
FORWARD_DECLARE_CLASS(RenderView);
FORWARD_DECLARE_CLASS(RenderInterface);

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

	// Adds the texture to the resource list
	void                         AddTexture(const Texture* pTexture);

	// Drag and drop functions
	virtual void                 dragEnterEvent(QDragEnterEvent *event) override;

	virtual void                 dragMoveEvent(QDragMoveEvent *event) override;

	virtual void                 dragLeaveEvent(QDragLeaveEvent *event) override;

	virtual void                 dropEvent(QDropEvent *event) override;

	// Catch resize events
	virtual void                 resizeEvent(QResizeEvent *event) override;

private slots:
	void                         OnTextureSelectorRowChanged();
private:

	// Current previewing texture
	Texture*                     mPreviewTexture;
	// The list of textures
	std::vector<Texture*>        mTextureList;
	// The RenderView where the texture will be previewed on
	RenderView*                  mTexturePreviewRV;
	// The RenderInterface used to preview images 
	RenderInterface*             mTexturePreviewRI; 

	Ui::TextureResourceEditor    mUi;

};

#endif // TEXTURERESOURCEEDITOR_H
