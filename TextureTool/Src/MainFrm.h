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

	TextureResourceEditor*          GetTextureResourceEditor() { return mTextureResourceEditor; }

	QMenuBar*                       GetMenuBar() { return mUi.mMenuBar; }

	QToolBar*                       GetToolBar(){ return mUi.mToolBar; };


private slots:
	void                            Update();

private:
	Ui::MainFrm                     mUi;

protected:

	virtual void                    resizeEvent(QResizeEvent *event) override;

	// The texture editor
	TextureResourceEditor*          mTextureResourceEditor;
};

#endif // TEXTURETOOLRENDERING_H
