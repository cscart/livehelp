#pragma once

#include <QTreeWidget>

class QTreeWidgetContext : public QTreeWidget
{
	Q_OBJECT

protected:
	virtual QString GetCopyText();

public:
	explicit QTreeWidgetContext(QWidget *parent = 0);
	~QTreeWidgetContext() {}

private slots:;
	void ShowContextMenu(const QPoint &pos);
	void CopySelectedToClipboard(const bool checked);

};
