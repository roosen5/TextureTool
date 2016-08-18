#ifndef TEXTURETOOLRENDERING_H
#define TEXTURETOOLRENDERING_H

#include <QtWidgets/QMainWindow>
#include "ui_MainFrm.h"

FORWARD_DECLARE_CLASS(RenderView);

class MainFrm : public QMainWindow
{
	Q_OBJECT
	typedef QMainWindow Base;

public:
	MainFrm(QWidget *parent = 0);
	~MainFrm();

	TextureResourceEditor*          GetTextureResourceEditor() { return mTextureResourceEditor; } // MATHIJS MAYBE DELETE

	// Gets the menu bar, other classes want to append on this
	QMenuBar*                       GetMenuBar() { return mUi.mMenuBar; }

	// Gets the toolbar, other classes want to append on this
	QToolBar*                       GetToolBar(){ return mUi.mToolBar; };

	// Sets the status bar under the mainfrm to pStatus, a small message to the user
	void                            SetStatusBar(const QString& pStatus);

	// this action exists in the mainfrm, but is setup in the TextureResouceEditor, so this is to expose the action
	QAction*						GetImportTextureAction() { return mUi.mImportTextureAction; }


	// Toggle between night mode
	void                            ToggleNightMode();

	private slots:
	// When the user triggers the action to exit the application (In the file menu)
	void                            OnExitApplicationClicked();

	// When the user presses the night mode icon
	void                            OnToggleNightModeActionClicked();

private:
	Ui::MainFrm                     mUi;

protected:

	virtual void                    resizeEvent(QResizeEvent *event) override;

	// The texture editor
	TextureResourceEditor*          mTextureResourceEditor;

	// Toggle that makes the entire application load a night mode stylesheet
	QAction*                        mToggleNightModeAction;
};

#endif // TEXTURETOOLRENDERING_H
