#ifndef TEXTURETOOLRENDERING_H
#define TEXTURETOOLRENDERING_H

#include <QtWidgets/QMainWindow>
#include "ui_TextureToolRendering.h"

FORWARD_DECLARE_CLASS(RenderView);

class MainFrm : public QMainWindow
{
	Q_OBJECT
	typedef QMainWindow Base;

public:
	MainFrm(QWidget *parent = 0);
	~MainFrm();
	RenderView* GetRenderInterface() { return mRenderInterface; }
private slots:
	void Update();
	void OnClearBtnClicked();
	void OnRenderPlaneBtnClicked();
	void OnBlitBtnClicked();

private:
	Ui::TextureToolRenderingClass mUi;
	RenderView* mRenderInterface;
protected:

	virtual void resizeEvent(QResizeEvent *event) override;

};

#endif // TEXTURETOOLRENDERING_H
