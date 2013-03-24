#include "stdafx.h"
#include "QVisitorInfo.h"

QVisitorInfo::QVisitorInfo(QWidget *parent)
:QTableWidget(parent)
{
	setMouseTracking(true);
}

QVisitorInfo::QVisitorInfo(int rows, int columns, QWidget *parent)
:QTableWidget(rows, columns, parent)
{
	setMouseTracking(true);
}


void QVisitorInfo::mouseMoveEvent(QMouseEvent *event)
{
	const QPoint &pt = event->pos();

	QTableWidgetItem *item = this->itemAt(pt);
	if (item != NULL)
	{
		if (IsValidUrl(item->text()))
		{
			setCursor(Qt::PointingHandCursor);
			return;
		}
	}

	setCursor(Qt::ArrowCursor);
}

void QVisitorInfo::mousePressEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton)
		return;

	const QPoint &pt = event->pos();
	QTableWidgetItem *item = this->itemAt(pt);
	if (item && IsValidUrl(item->text()))
	{
		const QUrl url(item->text());
		OpenUrl(url);
	}
}

bool QVisitorInfo::IsValidUrl(const QString &str)
{
	const QUrl url(str);

	if (url.scheme() == "")
		return false;

	if (url.host() == "")
		return false;

	return true;
}

bool QVisitorInfo::OpenUrl(const QUrl &url)
{
	QDesktopServices service;
	return service.openUrl(url);
}


