#include "stdafx.h"
#include "QTableWidgetContext.h"

QTableWidgetContext::QTableWidgetContext(QWidget *parent)
:QTableWidget(parent)
{
	setMouseTracking(true);

	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowContextMenu(QPoint)));
	setContextMenuPolicy(Qt::CustomContextMenu);
}

QTableWidgetContext::QTableWidgetContext(int rows, int columns, QWidget *parent)
:QTableWidget(rows, columns, parent)
{
	setMouseTracking(true);

	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowContextMenu(QPoint)));
	setContextMenuPolicy(Qt::CustomContextMenu);
}


void QTableWidgetContext::ShowContextMenu(const QPoint &pos)
{
	QAction *copy_action = new QAction(QIcon(":/lh3/Resources/editcopy.png"), QApplication::trUtf8("Copy", "LBL_COPY"), this);
	copy_action->setShortcut(QKeySequence("Ctrl+C"));

	connect(copy_action, SIGNAL(triggered(bool)), this, SLOT(CopySelectedToClipboard(bool)));
	
	QMenu menu("Shortcut actions", this);
	menu.addAction(copy_action);
	menu.exec(QCursor::pos());
}

void QTableWidgetContext::CopySelectedToClipboard(const bool checked)
{
	const QString text = GetContextString();
	
	QClipboard *clipboard = QApplication::clipboard();
	if (clipboard != NULL)
		clipboard->setText(text);
}

QString QTableWidgetContext::GetContextString()
{
  QItemSelectionModel * selection = selectionModel();
  QModelIndexList indexes = selection->selectedIndexes();

  if(indexes.size() < 1)
    return "";

	// sort by columns
	std::sort(indexes.begin(), indexes.end());

  // You need a pair of indexes to find the row changes
  QModelIndex previous = indexes.first();
  indexes.removeFirst();
  QString selected_text;
  QModelIndex current;
  Q_FOREACH(current, indexes)
  {
    QVariant data = model()->data(previous);
    QString text = data.toString();
    
		// item text
    selected_text.append(text);

		// new line
    if (current.row() != previous.row())
      selected_text.append(QLatin1Char('\n'));

		selected_text.append(QLatin1Char('\n'));

    previous = current;
  }

  // add last element
  selected_text.append(model()->data(current).toString());
  selected_text.append(QLatin1Char('\n'));

	return selected_text;
}