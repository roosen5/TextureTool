#ifndef TEXTURETOOLRENDERING_H
#define TEXTURETOOLRENDERING_H

#include <QtWidgets/QMainWindow>
#include "ui_TextureToolRendering.h"

class MainFrm : public QMainWindow
{
	Q_OBJECT

public:
	MainFrm(QWidget *parent = 0);
	~MainFrm();

private:
	Ui::TextureToolRenderingClass mUi;
};

#endif // TEXTURETOOLRENDERING_H
