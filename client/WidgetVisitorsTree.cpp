#include "stdafx.h"
#include "WidgetVisitorsTree.h"

cWidgetVisitorsTree::cWidgetVisitorsTree()
{
	ui.setupUi(this); 

	ui.visitorsTree->header()->hideSection(cWidgetVisitorsTree::Id);
}

void cWidgetVisitorsTree::ApplyLanguageLabels()
{
	ui.retranslateUi(this);

	for (int i = cVisitor::Idle; i < cVisitor::StatusCount; ++i)
		refresh_section(static_cast<cVisitor::Status>(i));
}


int cWidgetVisitorsTree::get_index(cVisitor::Status status)
{
	switch (status)
	{
		case cVisitor::Idle: return 0;
		case cVisitor::WaitingChat: return 2;
		case cVisitor::Chatting: return 4;
		case cVisitor::ChatRequestSent: return 6;
		case cVisitor::CharRequestDeclined: return 8;
	};

	return -1;
}

void cWidgetVisitorsTree::refresh_section(cVisitor::Status status)
{
	const int index = get_index(status);
	if (index < 0)
		return;

	QTreeWidgetItem *section = ui.visitorsTree->topLevelItem(index);
	const int itemsCount = section->childCount();

	const QString count_str = "(" + QString::number(itemsCount) + ")";
	section->setText(cWidgetVisitorsTree::Count, count_str);

}

void cWidgetVisitorsTree::cleanup_section(cVisitor::Status status)
{
	const int index = get_index(status);
	if (index < 0)
		return;

	QTreeWidgetItem *section = ui.visitorsTree->topLevelItem(index);
	section->takeChildren();

	refresh_section(status);
}

void cWidgetVisitorsTree::Cleanup()
{
	cVisitor::Status statuses[] = {cVisitor::Idle, cVisitor::WaitingChat, cVisitor::Chatting, cVisitor::ChatRequestSent, cVisitor::CharRequestDeclined};
	for (int i = 0; i < (sizeof(statuses) / sizeof(statuses[0])); ++i)
		cleanup_section(statuses[i]);
}


void cWidgetVisitorsTree::AddVisitor(const cVisitor &visitor, const bool expand_section)
{
	QMutexLocker locker(&mutex);

	const int index = get_index(visitor.GetStatus());
	if (index < 0)
		return;

	QTreeWidgetItem *section = ui.visitorsTree->topLevelItem(index);

	if (expand_section)
		section->setExpanded(true);

	const int itemsCount = section->childCount();

	QStringList list;
	list.push_back(visitor.GetCombinedName());
	list.push_back(QString::number(visitor.GetId()));
	QTreeWidgetItem *new_item = new QTreeWidgetItem(list);
	section->insertChild(itemsCount, new_item);

	// set status icon
	new_item->setIcon(0, visitor.GetStatusIcon());

	refresh_section(visitor.GetStatus());
}

void cWidgetVisitorsTree::UpdateVisitor(const cVisitor &old_visitor, const cVisitor &new_visitor)
{
	if (old_visitor.GetId() != new_visitor.GetId())
		return;

	const int index = get_index(old_visitor.GetStatus());
	if (index < 0)
		return;

	QTreeWidgetItem *section = ui.visitorsTree->topLevelItem(index);
	const int itemsCount = section->childCount();

	if (old_visitor.GetStatus() == new_visitor.GetStatus())
	{
		// update visitor name
		for (int i = 0; i < itemsCount; i++)
		{
			QTreeWidgetItem *item = section->child(i);
			const uint_t id = item->text(cWidgetVisitorsTree::Id).toULong();
			if (id == old_visitor.GetId())
			{
				item->setText(cWidgetVisitorsTree::Name, new_visitor.GetCombinedName());
				break;
			}
		}
	}
	else
	{
		// remove visitor from the old section
		const bool expand = RemoveVisitor(old_visitor);

		// add visitor to the new section
		AddVisitor(new_visitor, expand);
	}
}

bool cWidgetVisitorsTree::RemoveVisitor(const cVisitor &visitor)
{
	QMutexLocker locker(&mutex);

	bool is_section_expanded = false;

	const int index = get_index(visitor.GetStatus());
	if (index < 0)
		return is_section_expanded;

	QTreeWidgetItem *section = ui.visitorsTree->topLevelItem(index);
	const int itemsCount = section->childCount();

	is_section_expanded = section->isExpanded();

	for (int i = 0; i < itemsCount; i++)
	{
		QTreeWidgetItem *item = section->child(i);
		const uint_t id = item->text(cWidgetVisitorsTree::Id).toULong();
		if (id == visitor.GetId())
		{
			section->removeChild(item);
			break;
		}
	}

	refresh_section(visitor.GetStatus());

	return is_section_expanded;
}


void cWidgetVisitorsTree::itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	QMutexLocker locker(&mutex);

	if (item->childCount() > 0 || item->parent() == NULL)
		return;

	const uint_t id = item->text(cWidgetVisitorsTree::Id).toULong();
	if (id > 0)
	{
		OnVisitorOpen(id);
		return;
	}
}


void cWidgetVisitorsTree::showPopup(const QPoint &pos)
{
	if (get_selected_id() > 0)
	{
		QMenu menu("Visitor actions", this);
		menu.addAction(ui.actionOpen_Visitor);
		menu.addAction(ui.actionAssign_name);
		menu.exec(QCursor::pos());
	}
}

uint_t cWidgetVisitorsTree::get_selected_id() const
{
	QList<QTreeWidgetItem *> list = ui.visitorsTree->selectedItems();
	if (list.size() == 0)
		return 0;

	QTreeWidgetItem *item = list.takeFirst();
	if (item->childCount() > 0 || item->parent() == NULL)
		return 0;

	const uint_t id = item->text(cWidgetVisitorsTree::Id).toULong();
	return ((id > 0) ? id : 0);
}

void cWidgetVisitorsTree::openVisitorAction()
{
	const uint_t id = get_selected_id();
	if (id > 0)
		OnVisitorOpen(id);
}

void cWidgetVisitorsTree::renameVisitorAction()
{
	const uint_t id = get_selected_id();
	if (id > 0)
		OnVisitorRename(id);
}

/*
void cWidgetVisitorsTree::itemSelectionChanged()
{
	QList<QTreeWidgetItem *> list = ui.visitorsTree->selectedItems();
	if (list.size() == 0)
	{
		OnVisitorDeselect();
		return;
	}

	QTreeWidgetItem *item = list.takeFirst();
	if (item->childCount() > 0 || item->parent() == NULL)
	{
		OnVisitorDeselect();
		return;
	}

	const uint_t id = item->text(cWidgetVisitorsTree::Id).toULong();
	if (id > 0)
		OnVisitorSelect(id);
}
//*/


