#ifndef TEXTURERESOURCEEDITOR_H
#define TEXTURERESOURCEEDITOR_H

#include <QWidget>
#include "ui_TextureResourceEditor.h"

FORWARD_DECLARE_CLASS(Texture)
FORWARD_DECLARE_CLASS(RenderView)
FORWARD_DECLARE_CLASS(RenderInterface)
FORWARD_DECLARE_CLASS(MainFrm)
FORWARD_DECLARE_CLASS(SharpeningFilter)


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

	// Retrieve the spacer, mainfrm needs it to it to know where the spacing begins, and insert buttons before it
	QAction*                     GetRenderModeSpacer() { return mRenderModeSpacerAction; }
protected:
	void                         SetPreviewTexture(Texture* pPreviewTexture);

	// Syncs the selector to the resources
	void                         UpdateTextureSelector();

	// updatePreview mipmap combobox 
	void                         UpdateMipmapCB();

	// Openes explorer and selects the file
	void                         RevealCurrentTextureInExplorer();

	// Loads the texture, doesn't add it to the resource list
	HRESULT                      LoadTexture(const char* pFileName, Texture*& pTexture);

	// Update the render preview types (Render single channel or blended)
	void                         UpdateRenderPreviewTypeCB();

	// Create supported formats list
	void                         SetupSupportedTextureFormats();

	// Fills the menu with the rendering modes, and checks off which one has been selected
	void                         UpdateRenderingMenuAndToolbar();

	// All the conversion functions happen inside
	void                         ConvertTexture(DXGI_FORMAT pFormat, TextureEntry* pTextureEntry);

	// Create supported formats list
	void                         AddSupportedTextureFormat(DXGI_FORMAT pFormat, const QString& pFormatName);
	
	// Iterates through the supported texture list to find the format
	void                         SetSupportedTextureCBToFormat(DXGI_FORMAT pFormat);

	// When the loaded texture is not a power of 2, the texture will not be able to be compressed
	void                         SetCBCompressionTypesEnabled(bool pEnabled);

	void                         UpdateTextureInfo();
	// Adds the texture to the resource list
	TextureEntry*                AddTexture(const Texture* pTexture);

	// Returns the original if there's no converted texture
	Texture*                     GetPreviewingTexture();

	// Drag and drop functions
	virtual void                 dragEnterEvent(QDragEnterEvent *event) override;

	virtual void                 dragMoveEvent(QDragMoveEvent *event) override;

	virtual void                 dragLeaveEvent(QDragLeaveEvent *event) override;

	virtual void                 dropEvent(QDropEvent *event) override;

private slots:

// These actions can either be triggered in the toolbar, or in one of the the menu's
	// when the user triggers the shapening filter action
	void                         SharpeningFilterActionTriggered();

	// when the user triggers the reveal in folder action
	void                         RevealInExplorerActionTriggered();

	// when the user triggers the delete selected texture action
	void                         DeleteSelectedTextureActionTriggered();

	// when the user triggers the DiffuseAction
	void                         RenderDiffuseActionTriggered();

	// when the user triggers the NormalMap
	void                         RenderNormalMapActionTriggered();

	// when the user triggers the ImportTextureAction
	void                         ImportTextureActionTriggered();
//

	// Called when the user clicks on a different texture in the textureselector list
	void                         OnTextureSelectorRowChanged();

	// Catch changing mipmap user control
	void                         OnPreviewMipmapCBIndexChanged();

	// Catch when the render view is getting resized, because we have to render the texture again
	void                         OnTexturePreviewRVResized();

	// When the user presses the "Configure Sharpening Filter" button
	void                         OnConfigureSharpeningFilterBtnPressed();

	// When the user checks the "use sharpening filter" checkbox on/off, disable/enable the "Configure Sharpening Filter" button
	void                         OnUseSharpeningFilterCheckboxChanged();

	// Called when the user presses the "convert" button
	void                         OnConvertBtnPressed();

	// Catch when changed, pass the new preview type to the RenderView
	void                         OnRenderPreviewTypeCBIndexChanged();

	// Open catch double click event to reveal the file in the browser
	void                         OnTextureSelectorItemDoubleClicked(QListWidgetItem* pItem);

	// React to when the user right clicks on the item/requests the context menu
	void                         OnTextureSelectorContextMenuRequested();

	void                         OnCompressionTypeCBIndexChanged();

private:

	// The action that openes the sharpening filter dialog
	QAction*                     mConfigureSharpeningFilterAction;

	// The action that openes the filedialog, retrieved from mainfrm during setup
	QAction*                     mImportTextureAction;

	// The action that sets the renderer to use the diffuse shader
	QAction*                     mRenderDiffuseAction;

	// The action that sets the renderer to use the normalmap shader
	QAction*                     mRenderNormalMapAction;

	// The action that openes the explorer and reveals the selected texture
	QAction*                     mRevealInExplorerAction;

	// The action that deletes the currently selected texture entry
	QAction*                     mDeleteSelectedTextureAction;
	// A checkerboard texture, will be rendered to clear the texture preview screen, not using a color 
	// because we want to be able display the alpha
	Texture*                     mCheckerboardTexture;

	// The list of textures
	std::vector<TextureEntry*>   mTextureList;

	// The RenderView where the texture will be previewed on
	RenderView*                  mTexturePreviewRV;

	// The RenderInterface used to preview images 
	RenderInterface*             mTexturePreviewRI; 

	// The sharping filter, also acts as the editor
	SharpeningFilter*            mSharpeningFilter;

	// This action has a widget that acts as a spacer, since you can't add QSpacerItems to a toolbar
	QAction*                     mRenderModeSpacerAction; 

	// Ptr to the MainFrm, the main window/form
	MainFrm*					 mMainFrm;

	Ui::TextureResourceEditor    mUi;
};

#endif // TEXTURERESOURCEEDITOR_H
