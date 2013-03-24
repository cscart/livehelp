#pragma once

#include <QTableWidget>
#include <QDesktopServices>

class QVisitorInfo : public QTableWidget
{
	Q_OBJECT

private:
	bool IsValidUrl(const QString &str);
	bool OpenUrl(const QUrl &url);


protected:

	// override
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);

public:
	explicit QVisitorInfo(QWidget *parent = 0);
	QVisitorInfo(int rows, int columns, QWidget *parent = 0);
	~QVisitorInfo(void) {}


};
