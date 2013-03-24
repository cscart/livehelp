#include "stdafx.h"
#include "QTreeWidgetContext.h"

QTreeWidgetContext::QTreeWidgetContext(QWidget *parent)
:QTreeWidget(parent)
{
	connect(parent, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowContextMenu(QPoint)));
	parent->setContextMenuPolicy(Qt::CustomContextMenu);
}


void QTreeWidgetContext::ShowContextMenu(const QPoint &pos)
{
	QAction *copy_action = new QAction(QIcon(":/lh3/Resources/editcopy.png"), QApplication::trUtf8("Copy", "LBL_COPY"), this);
	copy_action->setShortcut(QKeySequence("Ctrl+C"));

	connect(copy_action, SIGNAL(triggered(bool)), this, SLOT(CopySelectedToClipboard(bool)));
	
	QMenu menu("Shortcut actions", this);
	menu.addAction(copy_action);
	menu.exec(QCursor::pos());
}

void QTreeWidgetContext::CopySelectedToClipboard(const bool checked)
{
	const QString text = GetCopyText();

	QClipboard *clipboard = QApplication::clipboard();
	if (clipboard != NULL)
		clipboard->setText(text);
}


QString QTreeWidgetContext::GetCopyText()
{
	QString result;

	QSet<int> columns;
	const int columns_count = columnCount();
	for (int i = 0; i < columns_count; ++i)
	{
		if (!header()->isSectionHidden(i))
			columns.insert(i);
	}

	QList<QTreeWidgetItem*> items = selectedItems();

	for (QList<QTreeWidgetItem*>::const_iterator it = items.begin(); it != items.end(); ++it)
	{
		QTreeWidgetItem *item = *it;

		for (QSet<int>::const_iterator ci = columns.begin(); ci != columns.end(); ++ci)
		{
			result += item->text(*ci);
			result += "\n";
		}

		result += "\n";
	}

	return result;
}