#include "stdafx.h"
#include "WidgetDockOfflineMessages.h"

CWidgetOfflineMessages::CWidgetOfflineMessages()
{
	ui.setupUi(this);
	ui.twOfflineMessages->header()->hideSection(ColumnId);

	dbMessages.Open();
}

CWidgetOfflineMessages::~CWidgetOfflineMessages()
{
}

void CWidgetOfflineMessages::fill_item(QTreeWidgetItem *const item, const COfflineMessageRecord &record)
{
	item->setText(ColumnSubject, record.GetSubject());
	item->setText(ColumnId, QString::number(record.GetId()));
	item->setText(ColumnVisitor, record.GetFromName());
	item->setText(ColumnDate, record.GetDateString());

	// mark all unread messages with Bold
	QFont font = item->font(ColumnSubject);
	font.setBold(record.IsUnread());

	int columns[] = {ColumnSubject, ColumnVisitor, ColumnDate};
	for (int i = 0; i < sizeof(columns) / sizeof(columns[0]); ++i)
		item->setFont(columns[i], font);

	item->setIcon(ColumnSubject, record.GetIcon());
}

void CWidgetOfflineMessages::display_item(QTreeWidgetItem *item)
{
	const uint_t id = item->text(ColumnId).toUInt();

	COfflineMessageRecord record(id);
	if (dbMessages.GetRecord(id, record))
	{
		record.SetUnread(false);
		OnRecordUpdate(record);

		// display Sticker in modal window
		auto_ptr<CDialogOfflineMsgView> dialog(new CDialogOfflineMsgView(record));
		dialog->exec();

		bool need_update = false;

		if (dialog->IsMarkUnread())
		{
			record.SetUnread(true);
			need_update = true;
		}

		if (dialog->IsReply())
		{
			record.SetStatus(COfflineMessageRecord::StatusPending);
			record.SetReply(dialog->GetReplyText());
			replyMessages.push_back(record);
			need_update = true;
		}

		if (need_update)
			OnRecordUpdate(record);
	}
}


void CWidgetOfflineMessages::UpdateFromXML(const CNodeXML &root)
{
	bool is_new_offline_message(false);

	CNodeXML nodes;
	if (root.GetNodeByPath("OfflineMessages", nodes) && nodes.GetNodesCount() > 0 && nodes.NodeStart())
	{
		do
		{
			CNodeXML node = nodes.GetNode();

			bool failed = false;

			QString id, date, from_name, from_email, subject, message, reply, reply_date, operator_id, operator_name, status, hash;

			if (!node.GetValueByPath("Id", id))
				failed = true;

			if (!node.GetValueByPath("Date", date))
				failed = true;

			if (!node.GetValueByPath("FromName", from_name))
				failed = true;

			if (!node.GetValueByPath("FromEmail", from_email))
				failed = true;

			if (!node.GetValueByPath("Subject", subject))
				failed = true;

			if (!node.GetValueByPath("Message", message))
				failed = true;

			if (!node.GetValueByPath("Reply", reply))
				failed = true;

			if (!node.GetValueByPath("ReplyDate", reply_date))
				failed = true;

			if (!node.GetValueByPath("OperatorId", operator_id))
				failed = true;

			if (!node.GetValueByPath("OperatorName", operator_name))
				failed = true;

			if (!node.GetValueByPath("Status", status))
				failed = true;

			if (!node.GetValueByPath("Hash", hash))
				failed = true;

			// interrupt if error found
			if (failed)
				return;

			const uint_t record_id = id.toUInt();
			COfflineMessageRecord record(record_id);

			record.SetDate(date.toUInt());
			record.SetFromName(from_name);
			record.SetFromEmail(from_email);
			record.SetSubject(subject);
			record.SetMessage(message);
			record.SetReply(reply);
			record.SetReplyDate(reply_date.toUInt());
			record.SetOperatorId(operator_id.toUInt());
			record.SetOperatorName(operator_name);
			record.SetStatus(static_cast<COfflineMessageRecord::Status>(status.toUInt()));
			record.SetHash(hash);

			OfflineMessagesArray::iterator it = messages.find(record_id);
			if (it != messages.end())
			{
				// do nothing if hash equal
				if ((*it).second.GetHash() == hash)
					continue;

				// erase old one
				messages.erase(it);
				OnRecordUpdate(record);
			}
			else
			{
				OnRecordAdd(record);
				is_new_offline_message |= true;
			}

			// insert new record
			messages.insert(std::pair<uint_t, COfflineMessageRecord>(record_id, record));

		} while (nodes.NodeNext());
	}

	// Obsolete stickers
	if (root.GetNodeByPath("ObsoleteOfflineMessages", nodes) && nodes.GetNodesCount() > 0 && nodes.NodeStart())
	{
		do
		{
			CNodeXML node = nodes.GetNode();

			uint_t id;

			if (node.HasAttribute("Id"))
				id = node.GetAttributeString("Id").toUInt();
			else
				continue;

			OfflineMessagesArray::iterator it = messages.find(id);
			if (it != messages.end())
			{
				messages.erase(it);
				OnRecordDelete(id);
			}

		} while (nodes.NodeNext());
	}
//*/

	if (is_new_offline_message)
		EventNewOfflineMessage();
}

void CWidgetOfflineMessages::GetMessagesDOM(QDomElement &root) const
{

	QDomDocument doc;

	QDomElement elem = doc.createElement("OfflineMessages");
	root.appendChild(elem);

	for (OfflineMessagesArray::const_iterator it = messages.begin(); it != messages.end(); ++it)
	{
		const COfflineMessageRecord &message = (*it).second;
		message.GetShortParamsDOM(elem);
	}

	// reply messages
	for (list<COfflineMessageRecord>::const_iterator it = replyMessages.begin(); it != replyMessages.end(); ++it)
	{
		const COfflineMessageRecord &message = *it;
		message.GetShortParamsDOM(elem, COfflineMessageRecord::FeedbackReply);
	}

	// obsolete messages
	for (list<COfflineMessageRecord>::const_iterator it = deletedMessages.begin(); it != deletedMessages.end(); ++it)
	{
		const COfflineMessageRecord &message = *it;
		message.GetShortParamsDOM(elem, COfflineMessageRecord::FeedbackObsolete);
	}

	replyMessages.clear();
	deletedMessages.clear();
}




void CWidgetOfflineMessages::OnRecordAdd(const COfflineMessageRecord &record)
{
	// insert DB row
	dbMessages.InsertUpdateRecord(record);

	// create and fill item
	QTreeWidgetItem *item = new QTreeWidgetItem();
	fill_item(item, record);

	// insert item into tree
	ui.twOfflineMessages->insertTopLevelItem(ui.twOfflineMessages->topLevelItemCount(), item);
}

void CWidgetOfflineMessages::OnRecordUpdate(const COfflineMessageRecord &record)
{
	// update DB row
	dbMessages.InsertUpdateRecord(record);

	// update Tree view
	for (int i = 0; i < ui.twOfflineMessages->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem *item = ui.twOfflineMessages->topLevelItem(i);
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

void CWidgetOfflineMessages::OnRecordDelete(const uint_t &record_id)
{
	// delete DB row
	dbMessages.DeleteRecord(record_id);

	// delete record from the TreeView
	for (int i = 0; i < ui.twOfflineMessages->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem *item = ui.twOfflineMessages->topLevelItem(i);
		if (item == NULL)
			continue;

		const QString id(item->text(ColumnId));
		if (id.toUInt() == record_id)
		{
			ui.twOfflineMessages->takeTopLevelItem(i);
			break;
		}
	}
}


void CWidgetOfflineMessages::InitFill()
{
	OfflineMessages records = dbMessages.GetRecords();
	for (OfflineMessages::iterator it = records.begin(); it != records.end(); ++it)
	{
		const COfflineMessageRecord &record = *it;
		OfflineMessagesArray::iterator it_temp = messages.find(record.GetId());
		if (it_temp != messages.end())
		{
			messages.erase(it_temp);
			messages.insert(std::pair<uint_t, COfflineMessageRecord>(record.GetId(), record));

			OnRecordUpdate(record);
			continue;
		}

		messages.insert(std::pair<uint_t, COfflineMessageRecord>(record.GetId(), record));
		OnRecordAdd(record);
	}
}

void CWidgetOfflineMessages::Cleanup()
{
	messages.clear();
	ui.twOfflineMessages->clear();
}

void CWidgetOfflineMessages::itemSelectionChanged()
{
	QList<QTreeWidgetItem*> items = ui.twOfflineMessages->selectedItems();
	if (items.size() <= 0)
	{
		ui.pbMessageDelete->setEnabled(false);
		ui.pbMessageView->setEnabled(false);
		return;
	}

	bool allow_view = items.size() == 1;

	ui.pbMessageDelete->setEnabled(true);
	ui.pbMessageView->setEnabled(allow_view);

}

void CWidgetOfflineMessages::ViewRecord()
{
	QList<QTreeWidgetItem*> items = ui.twOfflineMessages->selectedItems();
	if (items.size() != 1)
		return;

	QTreeWidgetItem *const item = items.first();
	display_item(item);
}

void CWidgetOfflineMessages::DeleteRecord()
{
	QList<QTreeWidgetItem*> items = ui.twOfflineMessages->selectedItems();

	// confirm sticker(s) delete
//	const QString header_msg = QString("Delete Message") + ((items.size() > 1) ? "s" : "");
//	if (QMessageBox::question(this, header_msg, "Are you sure?", "&Yes", "No", 0, 1, 0) != 0)
	if (DialogAreYouSure(this, tr("Delete Message(s)", "MSG_DELETE_MESSAGES")) != 0)
		return;
	
	for (int i = 0; i < items.size(); ++i)
	{
		const QTreeWidgetItem *const item = items[i];
		const uint_t id = item->text(ColumnId).toUInt();

                OfflineMessagesArray::iterator it = messages.find(id);
		if (it == messages.end())
			return;

		deletedMessages.push_back((*it).second);
		messages.erase(it);

		OnRecordDelete(id);
	}
}
