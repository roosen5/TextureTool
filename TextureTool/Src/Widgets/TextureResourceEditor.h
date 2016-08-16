#ifndef TEXTURERESOURCEEDITOR_H
#define TEXTURERESOURCEEDITOR_H

#include <QWidget>
#include "ui_TextureResourceEditor.h"

FORWARD_DECLARE_CLASS(Texture)
FORWARD_DECLARE_CLASS(RenderView)
FORWARD_DECLARE_CLASS(RenderInterface)
FORWARD_DECLARE_CLASS(MainFrm)

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

	// Sets up the member variables, and places the QActions in the toolbar and menubar
	void                         Setup(MainFrm* pMainFrm);

	// Clears the screen and renders the current preview image
	void                         RenderPreviewImage();
protected:
	void                         SetPreviewTexture(Texture* pPreviewTexture);

	// Syncs the selector to the resources
	void                         UpdateTextureSelector();

	// updatePreview mipmap combobox 
	void                         UpdateMipmapCB();

	// Loads the texture, doesn't add it to the resource list
	HRESULT                      LoadTexture(const char* pFileName, Texture*& pTexture);

	// Update the render preview types (Render single channel or blended)
	void                         UpdateRenderPreviewTypeCB();

	// Create supported formats list
	void                         SetupSupportedTextureFormats();

	// Fills the menu with the rendering modes, and checks off which one has been selected
	void                         UpdateRenderingMenu();

	// All the conversion functions happen inside
	void                         ConvertTexture(DXGI_FORMAT pFormat, TextureEntry* pTextureEntry);

	// Create supported formats list
	void                         AddSupportedTextureFormat(DXGI_FORMAT pFormat, const QString& pFormatName);
	
	// Iterates through the supported texture list to find the format
	void                         SetSupportedTextureCBToFormat(DXGI_FORMAT pFormat);

	void                         UpdateTextureInfo();
	// Adds the texture to the resource list
	TextureEntry*                AddTexture(const Texture* pTexture);

	Texture*                     GetPreviewingTexture();

	// Drag and drop functions
	virtual void                 dragEnterEvent(QDragEnterEvent *event) override;

	virtual void                 dragMoveEvent(QDragMoveEvent *event) override;

	virtual void                 dragLeaveEvent(QDragLeaveEvent *event) override;

	virtual void                 dropEvent(QDropEvent *event) override;

private slots:

	// when the user presses the render diffuse icon
	void                         RenderDiffuseActionTriggered();

	// when the user presses the render normalmap icon
	void                         RenderNormalMapActionTriggered();

	// Called when the user clicks on a different texture in the textureselector list
	void                         OnTextureSelectorRowChanged();

	// Catch changing mipmap user control
	void                         OnPreviewMipmapCBIndexChanged();

	// Catch when the render view is getting resized, because we have to render the texture again
	void                         OnTexturePreviewRVResized();
	// Called when the user presses the "convert" button
	void                         OnConvertBtnPressed();

	// Catch when changed, pass the new preview type to the RenderView
	void                         OnRenderPreviewTypeCBIndexChanged();

	// Open catch double click event to reveal the file in the browser
	void                         OnTextureSelectorItemDoubleClicked(QListWidgetItem* pItem);

	void                         OnCompressionTypeIndexChanged();

private:

	// The menu in the mainfrm's menubar that contains the render options
	QMenu*                       mRenderingMenu;

	// The action that sets the renderer to use the diffuse shader
	QAction*                     mRenderDiffuseAction;

	// The action that sets the renderer to use the normalmap shader
	QAction*                     mRenderNormalMapAction;

	// A checkerboard texture, will be rendered to clear the texture preview screen, not using a color 
	// because we want to be able display the alpha
	Texture*                     mCheckerboardTexture;

	// The list of textures
	std::vector<TextureEntry*>   mTextureList;

	// The RenderView where the texture will be previewed on
	RenderView*                  mTexturePreviewRV;

	// The RenderInterface used to preview images 
	RenderInterface*             mTexturePreviewRI; 

	// Ptr to the MainFrm, the main window/form
	MainFrm*					 mMainFrm;

	Ui::TextureResourceEditor    mUi;
};

#endif // TEXTURERESOURCEEDITOR_H
