#include "stdafx.h"
#include "DialogVisitorRename.h"

cDialogVisitorRename::cDialogVisitorRename(const QString &old_name)
{
	ui.setupUi(this);

	ui.leName->setText(old_name);
}

