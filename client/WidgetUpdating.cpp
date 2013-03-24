#include "stdafx.h"
#include "WidgetUpdating.h"

WidgetUpdating::WidgetUpdating(QWidget *widget)
:QWidget(widget), parent(widget)
{
	ui.setupUi(this);
	_resize();
}

void WidgetUpdating::paintEvent(QPaintEvent *pe)
{
	QBrush brush(Qt::white);
	QPainter painter(this);
	painter.fillRect(pe->rect(), brush);

	_resize();
}

void WidgetUpdating::_resize()
{
	// retrieve message with
	QFontMetrics fm(ui.label->font());
	const int label_width = fm.width(ui.label->text());

	// get icon rect
	const QRect rc_icon = ui.upating_icon->frameRect();

	// calc panel dimensions
	const int width = label_width + rc_icon.width() + 50;
	const int height = rc_icon.height() + 30;

	// get parent dimensions
	const QRect rc_parent = parent->frameGeometry();

	// calc panel offset
	const int dx = (rc_parent.width() - width) / 2;
	const int dy = (rc_parent.height() - height) / 2;

	// init panel geometry
	move(dx, dy);
	resize(width, height);
}