#include "stdafx.h"
#include "Stickers.h"


CStickerRecord::CStickerRecord(const uint_t _id)
:id(_id), operator_id(0), date(0), unread(true)
{}

QString CStickerRecord::GetDateString(const bool next_line) const
{
	QDateTime date_time;
	date_time.setTime_t(date);
	return date_time.toString(QString("MMMM d, yyyy") + QString((next_line) ? "\n" : " ") + QString("h:mm:ss ap"));
}

QIcon CStickerRecord::GetIcon() const
{
	const QString icon_path = IsUnread() ? ":/lh3/Resources/sticker_unread.png" : ":/lh3/Resources/sticker_read.png";
	return QIcon(icon_path);
}

void CStickerRecord::GetShortParamsDOM(QDomElement &root, const FeedbackState state) const
{
	QDomDocument doc;
	QDomElement elem_sticker = doc.createElement("Sticker");
	root.appendChild(elem_sticker);

	switch (state)
	{
		case FeedbackHash:
		{
			elem_sticker.setAttribute("Id", QString::number(id));
			elem_sticker.setAttribute("Type", "Hash");

			QDomElement elem_hash = doc.createElement("Hash");
			elem_hash.appendChild(doc.createTextNode(hash));
			elem_sticker.appendChild(elem_hash);
		}
		break;

		case FeedbackNew:
		{
			elem_sticker.setAttribute("Type", "New");

			// title field
			QDomElement elem_title = doc.createElement("Title");
			elem_title.appendChild(doc.createTextNode(title));
			elem_sticker.appendChild(elem_title);

			// message field
			QDomElement elem_message = doc.createElement("Message");
			elem_message.appendChild(doc.createTextNode(message));
			elem_sticker.appendChild(elem_message);
		}
		break;

		case FeedbackObsolete:
			elem_sticker.setAttribute("Id", QString::number(id));
			elem_sticker.setAttribute("Type", "Obsolete");
		break;
	};
}


bool CStickers::tables_initialized = false;

QSqlQuery CStickers::init_tables(QSqlDatabase &database)
{
	QSqlQuery query(database);

	query.exec("\
CREATE TABLE IF NOT EXISTS stickers (\
`shop_id` varchar(32) NOT NULL default '',\
`id` integer NOT NULL default 0,\
`operator_id` integer NOT NULL default 0,\
`operator_name` text NOT NULL default '',\
`date` integer NOT NULL default 0,\
`title` varchar(255) NOT NULL default '',\
`message` text NOT NULL default '',\
`unread` integer(1) NOT NULL default 1,\
`hash` varchar(32) NOT NULL default '',\
PRIMARY KEY(`shop_id`, `id`))");

	return query;
}


StickersRecords CStickers::GetRecords()
{
	// assign SQL query
	QSqlQuery query = QueryStart();
	query.prepare("SELECT id, operator_id, operator_name, date, title, message, unread, hash FROM stickers WHERE shop_id = :shop_id");

	query.bindValue(":shop_id", GetShopHash());

	// execute query
	QueryExecute(query);

	StickersRecords records;
	for (bool cond = query.first(); cond; cond = query.next())
	{
		CStickerRecord record(query.value(0).toUInt());
		record.operator_id = query.value(1).toUInt();
		record.operator_name = query.value(2).toString();
		record.date = query.value(3).toUInt();
		record.title = query.value(4).toString();
		record.message = query.value(5).toString();
		record.unread = query.value(6).toInt() > 0 ? true : false;
		record.hash = query.value(7).toString();

		records.push_back(record);
	}

	return records;
}


void CStickers::InsertUpdateRecord(const CStickerRecord &record)
{
	if (!IsValid())
		return;

	const uint_t id = record.GetId();

/*
	// assign SQL query
	QSqlQuery query = QueryStart();
	query.prepare("SELECT COUNT(id) FROM stickers WHERE shop_id = :shop_id AND id = :record_id");

	query.bindValue(":shop_id", GetShopHash());
	query.bindValue(":record_id", static_cast<long>(record.GetId()));

	// execute query
	QueryExecute(query);

	const bool is_record_update = query.first() && query.value(0).toUInt() > 0;
//*/

	QSqlQuery query = QueryStart();

/*
	if (is_record_update)
	{
		// update
		query.prepare("UPDATE stickers SET \
operator_id = :operator_id, \
operator_name = :operator_name, \
date = :date, \
title = :title, \
message = :message, \
hash = :hash, \
unread = :unread");// \
WHERE shop_id = :shop_id AND id = :record_id");
	}
	else
	{
		// insert
		query.prepare("INSERT INTO stickers (shop_id, id, operator_id, operator_name, date, title, message, hash) VALUES \
(:shop_id, :record_id, :operator_id, :operator_name, :date, :title, :message, :hash)");
	}
//*/


	query.prepare("REPLACE INTO stickers (shop_id, id, operator_id, operator_name, date, title, message, unread, hash) VALUES \
								(:shop_id, :record_id, :operator_id, :operator_name, :date, :title, :message, :unread, :hash)");

	query.bindValue(":shop_id", GetShopHash());
        query.bindValue(":record_id", static_cast<unsigned int>(id));
        query.bindValue(":operator_id", static_cast<unsigned int>(record.GetOperatorId()));
	query.bindValue(":operator_name", record.GetOperatorName());
        query.bindValue(":date", static_cast<unsigned int>(record.GetDate()));
	query.bindValue(":title", record.GetTitle());
	query.bindValue(":message", record.GetMessage());
	query.bindValue(":hash", record.GetHash());
	query.bindValue(":unread", record.IsUnread() ? 1 : 0);

	QueryExecute(query);

}

void CStickers::DeleteRecord(const uint_t id)
{
	if (!IsValid())
		return;

	// assign SQL query
	QSqlQuery query = QueryStart();
	query.prepare("DELETE FROM stickers WHERE shop_id = :shop_id AND id = :record_id");

	query.bindValue(":shop_id", GetShopHash());
        query.bindValue(":record_id", static_cast<unsigned int>(id));

	// execute query
	QueryExecute(query);
}

bool CStickers::GetRecord(const uint_t id, CStickerRecord &record)
{
	if (!IsValid())
		return false;

	// assign SQL query
	QSqlQuery query = QueryStart();
	query.prepare("SELECT id, operator_id, operator_name, date, title, message, unread, hash FROM stickers WHERE shop_id = :shop_id AND id = :record_id");

	query.bindValue(":shop_id", GetShopHash());
        query.bindValue(":record_id", static_cast<unsigned int>(id));

	// execute query
	QueryExecute(query);

	if (query.first())
	{
		record.id = query.value(0).toUInt();
		record.operator_id = query.value(1).toUInt();
		record.operator_name = query.value(2).toString();
		record.date = query.value(3).toUInt();
		record.title = query.value(4).toString();
		record.message = query.value(5).toString();
		record.unread = query.value(6).toInt() > 0 ? true : false;
		record.hash = query.value(7).toString();
		return true;
	}

	return false;
}


/*
void CStickers::UpdateFromXML(const CNodeXML &root)
{
	if (!IsValid())
		return;

	CNodeXML messages;
	if (root.GetNodeByPath("Stickers", messages) && messages.GetNodesCount() > 0 && messages.NodeStart())
	{
		do
		{
			CNodeXML message = messages.GetNode();

			bool failed = false;
//*/

/*
			QString id, from_id, to_id, date, direction, text;

			if (message.HasAttribute("Id"))
				id = message.GetAttributeString("Id");
			else
				failed = true;

			if (!message.GetValueByPath("FromId", from_id))
				failed = true;

			if (!message.GetValueByPath("ToId", to_id))
				failed = true;

			if (!message.GetValueByPath("Direction", direction))
				failed = true;

			if (!message.GetValueByPath("Date", date))
				failed = true;

			if (!message.GetValueByPath("Text", text))
				failed = true;

			// interrupt if error found
			if (failed)
				return;

			QSqlQuery query;
			query.prepare("INSERT INTO chat_log\
					(hash, id, date, from_id, to_id, direction, message) VALUES \
					(:hash, :id, :date, :visitor_id, :operator_id, :direction, :message)");
			query.bindValue(":hash", GetShopHash());
			query.bindValue(":id", id);
			query.bindValue(":date", date);
			query.bindValue(":from_id", from_id);
			query.bindValue(":to_id", to_id);
			query.bindValue(":direction", direction);
			query.bindValue(":message", text);
			const bool result = query.exec();
//*/

/*
		} while (messages.NodeNext());
	}
}
//*/

