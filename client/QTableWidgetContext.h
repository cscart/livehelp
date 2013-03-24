#pragma once

#include <QTableWidget>
#include <algorithm>

class QTableWidgetContext : public QTableWidget
{
	Q_OBJECT

protected:
	virtual QString GetContextString();

public:
	explicit QTableWidgetContext(QWidget *parent = 0);
	QTableWidgetContext(int rows, int columns, QWidget *parent = 0);
	~QTableWidgetContext() {}


private slots:;
	void ShowContextMenu(const QPoint &pos);

	void CopySelectedToClipboard(const bool checked);

};
