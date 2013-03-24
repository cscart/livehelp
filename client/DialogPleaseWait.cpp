#include "stdafx.h"
#include "DialogPleaseWait.h"

cDialogPleaseWait::cDialogPleaseWait(QWidget *_parent)
//:QDialog(_parent, Qt::SplashScreen), parent(_parent)
:QDialog(_parent, Qt::Dialog | Qt::CustomizeWindowHint), parent(_parent)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
        setWindowModality(Qt::ApplicationModal);

//	parent->setEnabled(false);

	bool x = connect(&timer, SIGNAL(timeout()), this, SLOT(timer_event()));
	timer.start(100);//(500, this, SLOT(event_timer()));
}

cDialogPleaseWait::~cDialogPleaseWait()
{
	timer.stop();
//	if (parent)
//		parent->setEnabled(true);
}

void cDialogPleaseWait::timer_event()
{
	static bool is_inc = true;
	const int inc_value = 10;
	QProgressBar *bar = ui.progressBar;

	if (is_inc)
		if (bar->value() + inc_value > bar->maximum())
		{
			bar->setValue(bar->maximum() - (bar->maximum() - bar->value() + inc_value));
			is_inc = !is_inc;

			bar->setInvertedAppearance(!is_inc);

			return;
		}
		else
			bar->setValue(bar->value() + inc_value);

	if (!is_inc)
		if (bar->value() - inc_value < bar->minimum())
		{
			bar->setValue(bar->minimum() + (bar->value() + inc_value - bar->maximum()));
			is_inc = !is_inc;

			bar->setInvertedAppearance(!is_inc);

			return;
		}
		else
			bar->setValue(bar->value() - inc_value);
}
