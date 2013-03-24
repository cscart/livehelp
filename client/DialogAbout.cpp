#include "stdafx.h"
#include "DialogAbout.h"

cDialogAbout::cDialogAbout()
{
	Ui::uiDialogAbout ui_template;
	ui_template.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
}

cDialogAbout::~cDialogAbout()
{
}
