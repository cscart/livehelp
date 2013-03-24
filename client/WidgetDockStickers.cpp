#include "stdafx.h"
#include "WidgetDockStickers.h"

CWidgetStickers::CWidgetStickers()
{
	ui.setupUi(this);

	ui.twStickers->header()->hideSection(ColumnId);

	dbStickers.Open();
}

CWidgetStickers::~CWidgetStickers()
{
}

void CWidgetStickers::OnEnable(const bool enabled)
{
	ui.wAllControls->setEnabled(enabled);
}

void CWidgetStickers::UpdateFromXML(const CNodeXML &root)
{
	CNodeXML nodes;
	if (root.GetNodeByPath("Stickers", nodes) && nodes.GetNodesCount() > 0 && nodes.NodeStart())
	{
		do
		{
			CNodeXML node = nodes.GetNode();

			bool failed = false;

			QString id, operator_id, date, operator_name, title, message, hash;

			if (!node.GetValueByPath("Id", id))
				failed = true;

			if (!node.GetValueByPath("OperatorId", operator_id))
				failed = true;

			if (!node.GetValueByPath("OperatorName", operator_name))
				failed = true;

			if (!node.GetValueByPath("Date", date))
				failed = true;

			if (!node.GetValueByPath("Title", title))
				failed = true;

			if (!node.GetValueByPath("Message", message))
				failed = true;

			if (!node.GetValueByPath("Hash", hash))
				failed = true;

			// interrupt if error found
			if (failed)
				return;

			const uint_t record_id = id.toUInt();
			CStickerRecord record(record_id);

			record.SetOperatorId(operator_id.toUInt());
			record.SetOperatorName(operator_name);
			record.SetDate(date.toUInt());
			record.SetTitle(title);
			record.SetMessage(message);
			record.SetHash(hash);

			StickersArray::iterator it = stickers.find(record_id);
			if (it != stickers.end())
			{
				// do nothing if hash equal
				if ((*it).second.GetHash() == hash)
					continue;

				// erase old one
				stickers.erase(it);
				OnRecordUpdate(record);
			}
			else
				OnRecordAdd(record);

			// insert new record
			stickers.insert(std::pair<uint_t, CStickerRecord>(record_id, record));

		} while (nodes.NodeNext());
	}


	// Obsolete stickers
	if (root.GetNodeByPath("ObsoleteStickers", nodes) && nodes.GetNodesCount() > 0 && nodes.NodeStart())
	{
		do
		{
			CNodeXML node = nodes.GetNode();

			uint_t id;

			if (node.HasAttribute("Id"))
				id = node.GetAttributeString("Id").toUInt();
			else
				continue;

			StickersArray::iterator it = stickers.find(id);
			if (it != stickers.end())
			{
				stickers.erase(it);
				OnRecordDelete(id);
			}

		} while (nodes.NodeNext());
	}

	TryUpdateEnd();
}

void CWidgetStickers::GetStickersDOM(QDomElement &root) const
{
	QDomDocument doc;

	QDomElement elem = doc.createElement("Stickers");
	root.appendChild(elem);

	for (StickersArray::const_iterator it = stickers.begin(); it != stickers.end(); ++it)
	{
		const CStickerRecord &sticker = (*it).second;
		sticker.GetShortParamsDOM(elem);
	}

	// new stickers
	for (list<CStickerRecord>::const_iterator it = addedStickers.begin(); it != addedStickers.end(); ++it)
	{
		const CStickerRecord &sticker = *it;
		sticker.GetShortParamsDOM(elem, CStickerRecord::FeedbackNew);
	}

	// obsolete stickers
	for (list<CStickerRecord>::const_iterator it = deletedStickers.begin(); it != deletedStickers.end(); ++it)
	{
		const CStickerRecord &sticker = *it;
		sticker.GetShortParamsDOM(elem, CStickerRecord::FeedbackObsolete);
	}

	addedStickers.clear();
	deletedStickers.clear();
}


void CWidgetStickers::OnRecordAdd(const CStickerRecord &record)
{
	// insert DB row
	dbStickers.InsertUpdateRecord(record);

	// create and fill item
	QTreeWidgetItem *item = new QTreeWidgetItem();
	fill_item(item, record);

	// insert item into tree
	ui.twStickers->insertTopLevelItem(ui.twStickers->topLevelItemCount(), item);
}

void CWidgetStickers::OnRecordUpdate(const CStickerRecord &record)
{
	// update DB row
	dbStickers.InsertUpdateRecord(record);

	// update Tree view
	for (int i = 0; i < ui.twStickers->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem *item = ui.twStickers->topLevelItem(i);
		if (item == NULL)
			continue;

		const QString id(item->text(ColumnId));
		if (id.toUInt() == record.GetId())
		{
			fill_item(item, record);
			break;
		}
	}
}

void CWidgetStickers::OnRecordDelete(const uint_t &record_id)
{
	// delete DB row
	dbStickers.DeleteRecord(record_id);

	// delete record from the TreeView
	for (int i = 0; i < ui.twStickers->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem *item = ui.twStickers->topLevelItem(i);
		if (item == NULL)
			continue;

		const QString id(item->text(ColumnId));
		if (id.toUInt() == record_id)
		{
			ui.twStickers->takeTopLevelItem(i);
			break;
		}
	}
}


void CWidgetStickers::fill_item(QTreeWidgetItem *const item, const CStickerRecord &record)
{
	item->setText(ColumnTitle, record.GetTitle());
	item->setText(ColumnId, QString::number(record.GetId()));
	item->setText(ColumnOperator, record.GetOperatorName());
	item->setText(ColumnDate, record.GetDateString());

#ifdef STICKER_UNREAD_MARK_BOLD
	QFont font = item->font(ColumnTitle);
	font.setBold(record.IsUnread());

	int columns[] = {ColumnTitle, ColumnOperator, ColumnDate};
	for (int i = 0; i < sizeof(columns) / sizeof(columns[0]); ++i)
		item->setFont(columns[i], font);
#endif

#ifdef STICKER_UNREAD_MARK_ICON
	item->setIcon(ColumnTitle, record.GetIcon());
#endif
}

void CWidgetStickers::display_item(QTreeWidgetItem *item)
{
	const uint_t id = item->text(ColumnId).toUInt();

	CStickerRecord record(id);
	if (dbStickers.GetRecord(id, record))
	{
		record.SetUnread(false);
		OnRecordUpdate(record);

		// display Sticker in modal window
		auto_ptr<CDialogStickerView> dialog(new CDialogStickerView(record));
		dialog->exec();

		if (dialog->IsMarkUnread())
		{
			record.SetUnread(true);
			OnRecordUpdate(record);
		}
	}
}

void CWidgetStickers::InitFill()
{
	StickersRecords records = dbStickers.GetRecords();
	for (StickersRecords::iterator it = records.begin(); it != records.end(); ++it)
	{
		const CStickerRecord &record = *it;
		StickersArray::iterator it_temp = stickers.find(record.GetId());
		if (it_temp != stickers.end())
		{
			stickers.erase(it_temp);
			stickers.insert(std::pair<uint_t, CStickerRecord>(record.GetId(), record));

			OnRecordUpdate(record);
			continue;
		}

		stickers.insert(std::pair<uint_t, CStickerRecord>(record.GetId(), record));
		OnRecordAdd(record);
	}
}

void CWidgetStickers::Cleanup()
{
	stickers.clear();
	ui.twStickers->clear();
}


void CWidgetStickers::OnConnected()
{
	ui.pbStickerAdd->setEnabled(true);
}

void CWidgetStickers::OnDisconnected()
{
	ui.pbStickerAdd->setEnabled(false);
	IBehaviourUpdating::UpdateEnd();
}

void CWidgetStickers::itemSelectionChanged()
{
	QList<QTreeWidgetItem*> items = ui.twStickers->selectedItems();
	if (items.size() <= 0)
	{
		ui.pbStickerDelete->setEnabled(false);
		ui.pbStickerView->setEnabled(false);
		return;
	}

	bool allow_delete = true;
	bool allow_view = items.size() == 1;

	if (items.size() == 1)
		allow_delete = is_owner(items.first());

	ui.pbStickerDelete->setEnabled(allow_delete);
	ui.pbStickerView->setEnabled(allow_view);

}

void CWidgetStickers::ViewRecord()
{
	QList<QTreeWidgetItem*> items = ui.twStickers->selectedItems();
	if (items.size() != 1)
		return;

	QTreeWidgetItem *const item = items.first();
	display_item(item);
}

void CWidgetStickers::AddRecord()
{
	auto_ptr<CDialogStickerCreate> dialog(new CDialogStickerCreate);

	if (dialog->exec() == QDialog::Accepted)
	{
		CStickerRecord record(-1);
		record.SetOperatorName(cGlobals::GetAssignedOperatorName());
		record.SetDate(QDateTime::currentDateTime().toTime_t());
		record.SetTitle(dialog->GetTitle());
		record.SetMessage(dialog->GetMessage());
		record.SetUnread(false);

//		OnRecordAdd(record);
		addedStickers.push_back(record);

		UpdateBegin(this, 2);
	}
}

void CWidgetStickers::DeleteRecord()
{
	QList<QTreeWidgetItem*> items = ui.twStickers->selectedItems();

	// confirm sticker(s) delete
//	const QString header_msg = QString("Delete Sticker") + ((items.size() > 1) ? "s" : "");
//	if (QMessageBox::question(this, header_msg, "Are you sure?", "&Yes", "No", 0, 1, 0) != 0)
	if (DialogAreYouSure(this, tr("Delete Sticker(s)", "MSG_DELETE_STICKERS")) != 0)
		return;
	
	for (int i = 0; i < items.size(); ++i)
	{
		const QTreeWidgetItem *const item = items[i];
		const uint_t id = item->text(ColumnId).toUInt();

		StickersArray::iterator it = stickers.find(id);
		if (it == stickers.end())
			return;

		if (!is_owner(item))
			continue;

		deletedStickers.push_back((*it).second);
		stickers.erase(it);

		OnRecordDelete(id);

		UpdateBegin(this, 2);
	}
}

bool CWidgetStickers::is_owner(const QTreeWidgetItem *const item)
{
	assert(item);
	const uint_t id = item->text(ColumnId).toUInt();

	StickersArray::const_iterator it = stickers.find(id);
	if (it == stickers.end())
		return false;

	const CStickerRecord &record = (*it).second;
	return record.GetOperatorId() == cGlobals::GetOperatorId();
}
