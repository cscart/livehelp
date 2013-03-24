#pragma once

#include <QFontMetrics>

#include "ui_WidgetUpdating.h"

class WidgetUpdating : public QWidget
{
	Q_OBJECT

private:
	Ui_WidgetUpdating ui;

	QWidget *parent;

protected:
	// overload
	virtual void paintEvent(QPaintEvent *e);

	void _resize();

public:
	WidgetUpdating(QWidget *widget);
	~WidgetUpdating() {}
};
