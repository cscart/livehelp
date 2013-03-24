#include "stdafx.h"
#include "OfflineMessages.h"

COfflineMessageRecord::COfflineMessageRecord(const uint_t _id)
:id(_id), operator_id(0), date(0), reply_date(0), status(StatusNew), unread(true)
{}

QString COfflineMessageRecord::GetDateString(const bool next_line) const
{
	QDateTime date_time;
	date_time.setTime_t(date);
	return date_time.toString(QString("MMMM d, yyyy") + QString((next_line) ? "\n" : " ") + QString("h:mm:ss ap"));
}

QString COfflineMessageRecord::GetReplyDateString(const bool next_line) const
{
	QDateTime date_time;
	date_time.setTime_t(reply_date);
	return date_time.toString(QString("MMMM d, yyyy") + QString((next_line) ? "\n" : " ") + QString("h:mm:ss ap"));
}

void COfflineMessageRecord::GetShortParamsDOM(QDomElement &root, const FeedbackState state) const
{
	QDomDocument doc;
	QDomElement elem_message = doc.createElement("Message");
	root.appendChild(elem_message);

	switch (state)
	{
		case FeedbackHash:
		{
			elem_message.setAttribute("Id", QString::number(id));
			elem_message.setAttribute("Type", "Hash");

			QDomElement elem_hash = doc.createElement("Hash");
			elem_hash.appendChild(doc.createTextNode(hash));
			elem_message.appendChild(elem_hash);
		}
		break;

		case FeedbackReply:
		{
			elem_message.setAttribute("Id", QString::number(id));
			elem_message.setAttribute("Type", "Reply");

			// reply field
			QDomElement elem_reply = doc.createElement("Reply");
			elem_reply.appendChild(doc.createTextNode(reply));
			elem_message.appendChild(elem_reply);

			// subject field
			QDomElement elem_subject = doc.createElement("Subject");
			elem_subject.appendChild(doc.createTextNode(GetReplySubject()));
			elem_message.appendChild(elem_subject);

			// to-email field
			QDomElement elem_email = doc.createElement("ToEmail");
			elem_email.appendChild(doc.createTextNode(from_email));
			elem_message.appendChild(elem_email);
		}
		break;

		case FeedbackObsolete:
			elem_message.setAttribute("Id", QString::number(id));
			elem_message.setAttribute("Type", "Obsolete");
		break;
	};
}

QString COfflineMessageRecord::GetStatusString() const
{
	switch (status)
	{
	case StatusPending: return QApplication::trUtf8("Pending", "MSG_PENDING");
		case StatusReplied: return QApplication::trUtf8("Yes", "MSG_YES");
	};

	return QApplication::trUtf8("No", "MSG_NO");
}

QIcon COfflineMessageRecord::GetIcon() const
{
	QString icon_path;

	if (status == StatusNew)
		icon_path = IsUnread() ? ":/lh3/Resources/message_new.png" : ":/lh3/Resources/message_read.png";
	else
		icon_path = (status == StatusPending) ? ":/lh3/Resources/message_pending.png" : ":/lh3/Resources/message_sent.png";

	return QIcon(icon_path);
}



bool COfflineMessages::tables_initialized = false;

QSqlQuery COfflineMessages::init_tables(QSqlDatabase &database)
{
	QSqlQuery query(database);

	query.exec("\
CREATE TABLE IF NOT EXISTS offline_messages (\
`shop_id` varchar(32) NOT NULL default '',\
`id` integer NOT NULL default 0,\
`date` integer NOT NULL default 0,\
`from_name` text NOT NULL default '',\
`from_email` text NOT NULL default '',\
`subject` text NOT NULL default '',\
`message` text NOT NULL default '',\
`reply` text NOT NULL default '',\
`reply_date` integer NOT NULL default 0,\
`operator_id` integer NOT NULL default 0,\
`operator_name` text NOT NULL default '',\
`status` integer(1) NOT NULL default 0, \
`unread` integer(1) NOT NULL default 1,\
`hash` varchar(32) NOT NULL default '',\
PRIMARY KEY(`shop_id`, `id`))");

	return query;
}

OfflineMessages COfflineMessages::GetRecords()
{
	// assign SQL query
	QSqlQuery query = QueryStart();
	query.prepare("SELECT id, date, from_name, from_email, subject, message, reply, reply_date, \
operator_id, operator_name, status, unread, hash FROM offline_messages WHERE shop_id = :shop_id");

	query.bindValue(":shop_id", GetShopHash());

	// execute query
	QueryExecute(query);

	OfflineMessages records;
	for (bool cond = query.first(); cond; cond = query.next())
	{
		COfflineMessageRecord record(query.value(0).toUInt());
		record.date = query.value(1).toUInt();
		record.from_name = query.value(2).toString();
		record.from_email = query.value(3).toString();
		record.subject = query.value(4).toString();
		record.message = query.value(5).toString();
		record.reply = query.value(6).toString();
		record.reply_date = query.value(7).toUInt();
		record.operator_id = query.value(8).toUInt();
		record.operator_name = query.value(9).toString();
		record.status = static_cast<COfflineMessageRecord::Status>(query.value(10).toUInt());
		record.unread = query.value(11).toInt() > 0 ? true : false;
		record.hash = query.value(12).toString();

		records.push_back(record);
	}

	return records;
}


void COfflineMessages::InsertUpdateRecord(const COfflineMessageRecord &record)
{
	if (!IsValid())
		return;

	const uint_t id = record.GetId();

	// assign SQL query
	QSqlQuery query = QueryStart();
	query.prepare("SELECT COUNT(id) FROM offline_messages WHERE shop_id = :shop_id AND id = :record_id");

	query.bindValue(":shop_id", GetShopHash());
        query.bindValue(":record_id", static_cast<unsigned int>(record.GetId()));

	// execute query
	QueryExecute(query);

//	const bool is_record_update = query.first() && query.value(0).toUInt() > 0;

	query = QueryStart();
	query.prepare("REPLACE INTO offline_messages (shop_id, id, date, from_name, from_email, subject, message, reply, reply_date, \
operator_id, operator_name, status, unread, hash) VALUES \
(:shop_id, :record_id, :date, :from_name, :from_email, :subject, :message, :reply, :reply_date, \
:operator_id, :operator_name, :status, :unread, :hash)");

	query.bindValue(":shop_id", GetShopHash());
        query.bindValue(":record_id", static_cast<unsigned int>(id));
        query.bindValue(":date", static_cast<unsigned int>(record.GetDate()));
	query.bindValue(":from_name", record.GetFromName());
	query.bindValue(":from_email", record.GetFromEmail());
	query.bindValue(":subject", record.GetSubject());
	query.bindValue(":message", record.GetMessage());
	query.bindValue(":reply", record.GetReply());
        query.bindValue(":reply_date", static_cast<unsigned int>(record.GetReplyDate()));
        query.bindValue(":operator_id", static_cast<unsigned int>(record.GetOperatorId()));
	query.bindValue(":operator_name", record.GetOperatorName());
        query.bindValue(":status", static_cast<unsigned int>(record.GetStatus()));
	query.bindValue(":unread", record.IsUnread() ? 1 : 0);
	query.bindValue(":hash", record.GetHash());

	QueryExecute(query);
}

void COfflineMessages::DeleteRecord(const uint_t id)
{
	if (!IsValid())
		return;

	// assign SQL query
	QSqlQuery query = QueryStart();
	query.prepare("DELETE FROM offline_messages WHERE shop_id = :shop_id AND id = :record_id");

	query.bindValue(":shop_id", GetShopHash());
        query.bindValue(":record_id", static_cast<unsigned int>(id));

	// execute query
	QueryExecute(query);
}

bool COfflineMessages::GetRecord(const uint_t id, COfflineMessageRecord &record)
{
	if (!IsValid())
		return false;

	// assign SQL query
	QSqlQuery query = QueryStart();
	query.prepare("SELECT id, date, from_name, from_email, subject, message, reply, reply_date, \
operator_id, operator_name, status, unread, hash FROM offline_messages WHERE shop_id = :shop_id AND id = :record_id");

	query.bindValue(":shop_id", GetShopHash());
        query.bindValue(":record_id", static_cast<unsigned int>(id));

	// execute query
	QueryExecute(query);

	if (query.first())
	{
		record.id = query.value(0).toUInt();
		record.date = query.value(1).toUInt();
		record.from_name = query.value(2).toString();
		record.from_email = query.value(3).toString();
		record.subject = query.value(4).toString();
		record.message = query.value(5).toString();
		record.reply = query.value(6).toString();
		record.reply_date = query.value(7).toUInt();
		record.operator_id = query.value(8).toUInt();
		record.operator_name = query.value(9).toString();
		record.status = static_cast<COfflineMessageRecord::Status>(query.value(10).toUInt());
		record.unread = query.value(11).toInt() > 0 ? true : false;
		record.hash = query.value(12).toString();
		return true;
	}

	return false;
}
