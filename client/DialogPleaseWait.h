#pragma once

#include <QTimer>
#include "ui_DialogPleaseWait.h"

class cDialogPleaseWait : public QDialog
{
	Q_OBJECT

private:
	Ui::uiDialogPleaseWait ui;

	QWidget *parent;

	QTimer timer;

public:
	cDialogPleaseWait(QWidget *_parent);
	~cDialogPleaseWait();


private slots:;

	void timer_event();

};
