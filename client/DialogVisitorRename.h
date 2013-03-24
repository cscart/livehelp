#pragma once

#include "ui_DialogVisitorRename.h"

class cDialogVisitorRename : public QDialog
{
	Q_OBJECT

private:
	Ui::uiDialogVisitorRename ui;

public:
	cDialogVisitorRename(const QString &old_name);
	~cDialogVisitorRename() {}

	QString GetName() const { return ui.leName->text(); }
};
