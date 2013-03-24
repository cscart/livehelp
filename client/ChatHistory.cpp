#include "stdafx.h"
#include "ChatHistory.h"

CChatHistoryRecord::CChatHistoryRecord(uint_t _id, uint_t _date, Direction _dir, const QString &_text)
:id(_id), date(_date), direction(_dir), text(_text), matched(false)
{
}

QString CChatHistoryRecord::GetDateString() const
{
	QDateTime date_time;
	date_time.setTime_t(date);
	return date_time.toString("MMMM d, yyyy\nh:mm:ss ap");
}


bool CChatHistory::tables_initialized = false;


QSqlQuery CChatHistory::init_tables(QSqlDatabase &database)
{
	QSqlQuery query(database);

	query.exec("\
CREATE TABLE IF NOT EXISTS chat_log (\
`hash` varchar(32) NOT NULL default '',\
`id` integer NOT NULL default 0,\
`date` integer NOT NULL default 0,\
`from_id` integer NOT NULL default 0,\
`to_id` integer NOT NULL default 0,\
`direction` integer(1) NOT NULL default 0,\
`message` text NOT NULL default '',\
`visible` integer(1) NOT NULL default 1,\
PRIMARY KEY(`hash`, `id`))");

	return query;
}

QString CChatHistory::get_sql_conditions(QMap<QString, QVariant> &query_values, const uint_t visitor_id, const uint_t operator_id/*, const QString &SearchString*/) const
{
	QString sql("hash = :hash AND \
((from_id = :visitor_id1 AND to_id = :operator_id1 AND direction = 0) OR \
(from_id = :operator_id2 AND to_id = :visitor_id2 AND direction = 1)) AND visible = 1");

	query_values.insert(":hash", GetShopHash());
	query_values.insert(":visitor_id1", static_cast<unsigned int>(visitor_id));
	query_values.insert(":operator_id1", static_cast<unsigned int>(operator_id));
	query_values.insert(":visitor_id2", static_cast<unsigned int>(visitor_id));
	query_values.insert(":operator_id2", static_cast<unsigned int>(operator_id));

/*
	const QString search_pattern(SearchString.trimmed());
	if (search_pattern.length() > 0)
	{
		sql += " AND message LIKE(:search_pattern)";
		query_values.insert(":search_pattern", "%" + search_pattern + "%");
	}
//*/

	return sql;
}

ChatHistoryRecords CChatHistory::GetRecords(const uint_t visitor_id, const uint_t operator_id, const QString &SearchString, const int page)
{
	QMap<QString, QVariant> query_values;

	const QString conditions(get_sql_conditions(query_values, visitor_id, operator_id/*, SearchString*/));

	QString sql("SELECT id, direction, message, date FROM chat_log WHERE " + conditions);

	// order condition
	sql += " ORDER BY id DESC";

	// pager condition
	const uint_t start = page * CHAT_LOG_MESSAGES_PER_PAGE;
	const uint_t count = CHAT_LOG_MESSAGES_PER_PAGE;

	sql += " LIMIT :start, :length";

	query_values.insert(":start", static_cast<unsigned int>(start));
	query_values.insert(":length", static_cast<unsigned int>(count));

	// assign SQL query
	QSqlQuery query = QueryStart();
	query.prepare(sql);

	// apply key-value pairs
	for (QMap<QString, QVariant>::const_iterator it = query_values.begin(); it != query_values.end(); ++it)
		query.bindValue(it.key(), it.value());

	// execute query
	QueryExecute(query);


	// find pattern matches
	set<uint_t> found_ids;
	const QString search_pattern(SearchString.trimmed());
	if (search_pattern.length() > 0)
	{
		QSqlQuery mquery = QueryStart();
		mquery.prepare("SELECT id FROM chat_log WHERE hash = :hash AND \
((from_id = :visitor_id1 AND to_id = :operator_id1 AND direction = 0) OR \
(from_id = :operator_id2 AND to_id = :visitor_id2 AND direction = 1)) AND visible = 1 AND message LIKE(:search_pattern)");
		mquery.bindValue(":hash", GetShopHash());
		mquery.bindValue(":visitor_id1", static_cast<unsigned int>(visitor_id));
		mquery.bindValue(":operator_id1", static_cast<unsigned int>(operator_id));
		mquery.bindValue(":visitor_id2", static_cast<unsigned int>(visitor_id));
		mquery.bindValue(":operator_id2", static_cast<unsigned int>(operator_id));
		mquery.bindValue(":search_pattern", "%" + search_pattern + "%");

		QueryExecute(mquery);

		if (mquery.first())
		{
			do
			{
				uint_t id = mquery.value(0).toUInt();
				found_ids.insert(id);
			} while (mquery.next());

		}
	}


	// traverse all rows
	ChatHistoryRecords messages;
	if (query.first())
	{
		do
		{
			const uint_t id = query.value(0).toUInt();
			CChatHistoryRecord message(id, query.value(3).toInt(),
				static_cast<CChatHistoryRecord::Direction>(query.value(1).toInt()), query.value(2).toString());

			if (found_ids.find(id) != found_ids.end())
				message.matched = true;

			messages.push_back(message);
		} while (query.next());
	}

	return messages;
}

uint_t CChatHistory::GetRecordsCount(const uint_t visitor_id, const uint_t operator_id/*, const QString &SearchString*/)
{
	QMap<QString, QVariant> query_values;

	const QString conditions(get_sql_conditions(query_values, visitor_id, operator_id/*, SearchString*/));

	const QString sql("SELECT COUNT(id) FROM chat_log WHERE " + conditions);
	
	// assign SQL query
	QSqlQuery query = QueryStart();
	query.prepare(sql);

	// apply key-value pairs
	for (QMap<QString, QVariant>::const_iterator it = query_values.begin(); it != query_values.end(); ++it)
		query.bindValue(it.key(), it.value());

	// execute query
	QueryExecute(query);

	return (query.first()) ? query.value(0).toInt() : 0;
}

long CChatHistory::GetLastMessageId(const uint_t visitor_id)
{
	const uint_t operator_id(cGlobals::GetOperatorId());
	return get_last_message_id(GetShopHash(), visitor_id, operator_id);
}

long CChatHistory::get_last_message_id(const QString &shop_id, const uint_t visitor_id, const uint_t operator_id, const uint_t date)
{
	if (!IsValid())
		return -1;

//	const uint_t operator_id(cGlobals::GetOperatorId());

	QSqlQuery query = QueryStart();
	query.prepare("SELECT MAX(id) FROM chat_log WHERE hash = :hash AND \
((from_id = :visitor_id1 AND to_id = :operator_id1 AND direction = 0) OR \
(from_id = :operator_id2 AND to_id = :visitor_id2 AND direction = 1))" + ((date > 0) ? " AND date < " + QString::number(date) : ""));
	query.bindValue(":hash", shop_id);
	query.bindValue(":visitor_id1", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id1", static_cast<unsigned int>(operator_id));
	query.bindValue(":visitor_id2", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id2", static_cast<unsigned int>(operator_id));
	QueryExecute(query);

	// return 'empty' if SQL failed
	const QSqlError error = query.lastError();
	if (error.type() != QSqlError::NoError)
		return -1;

	if (query.first())
		return query.value(0).toInt();

	return 0;
}

void CChatHistory::UpdateFromXML(const CNodeXML &root)
{
	if (!IsValid())
		return;

	CNodeXML messages;
	if (root.GetNodeByPath("ChatMessagesLog", messages) && messages.GetNodesCount() > 0 && messages.NodeStart())
	{
		do
		{
			CNodeXML message = messages.GetNode();

			bool failed = false;
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

		} while (messages.NodeNext());
	}
}

bool CChatHistory::ClearRecords(const uint_t visitor_id)
{
	const long last_id = GetLastMessageId(visitor_id);
	if (last_id <= 0)
		return false;

	return clear_records(GetShopHash(), visitor_id, cGlobals::GetOperatorId(), last_id);
}

bool CChatHistory::clear_records(const QString &shop_id, const uint_t visitor_id, const uint_t operator_id, const uint_t last_id)
{
	if (shop_id == "")
		return false;

//	const uint_t operator_id(cGlobals::GetOperatorId());

	QSqlQuery query = QueryStart();

	query.prepare("DELETE FROM chat_log WHERE hash = :hash AND id < :id AND \
((from_id = :visitor_id1 AND to_id = :operator_id1 AND direction = 0) OR \
(from_id = :operator_id2 AND to_id = :visitor_id2 AND direction = 1))");
	query.bindValue(":hash", shop_id);
	query.bindValue(":id", static_cast<unsigned int>(last_id));
	query.bindValue(":visitor_id1", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id1", static_cast<unsigned int>(operator_id));
	query.bindValue(":visitor_id2", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id2", static_cast<unsigned int>(operator_id));
	QueryExecute(query);

	query.prepare("UPDATE chat_log SET visible = 0 WHERE hash = :hash AND id = :id AND \
((from_id = :visitor_id1 AND to_id = :operator_id1 AND direction = 0) OR \
(from_id = :operator_id2 AND to_id = :visitor_id2 AND direction = 1))");
	query.bindValue(":hash", shop_id);
	query.bindValue(":id", static_cast<unsigned int>(last_id));
	query.bindValue(":visitor_id1", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id1", static_cast<unsigned int>(operator_id));
	query.bindValue(":visitor_id2", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id2", static_cast<unsigned int>(operator_id));
	QueryExecute(query);

	return true;
}

bool CChatHistory::ClearAll(const uint_t date)
{
	// forward direction
	QSqlQuery query = QueryStart();
	query.prepare("SELECT MAX(id) AS id, hash, from_id, to_id FROM chat_log WHERE direction = 0 GROUP BY hash, to_id, from_id");
	QueryExecute(query);

	// return 'false' if SQL failed
	const QSqlError error = query.lastError();
	if (error.type() != QSqlError::NoError)
		return false;

	if (query.first())
	{
		do
		{
			const uint_t id = query.value(0).toUInt();
			const QString shop_id = query.value(1).toString();
			const uint_t visitor_id = query.value(2).toUInt();
			const uint_t operator_id = query.value(3).toUInt();

			const uint_t last_id = get_last_message_id(shop_id, visitor_id, operator_id, date);
			if (last_id > 0)
				clear_records(shop_id, visitor_id, operator_id, last_id);

		} while (query.next());
	}


	// reverse direction
	query.prepare("SELECT MAX(id) AS id, hash, from_id, to_id FROM chat_log WHERE direction = 1 GROUP BY hash, to_id, from_id");
	QueryExecute(query);

	// return 'false' if SQL failed
	const QSqlError error2 = query.lastError();
	if (error2.type() != QSqlError::NoError)
		return false;

	if (query.first())
	{
		do
		{
			const uint_t id = query.value(0).toUInt();
			const QString shop_id = query.value(1).toString();
			const uint_t operator_id = query.value(2).toUInt();
			const uint_t visitor_id = query.value(3).toUInt();

			const uint_t last_id = get_last_message_id(shop_id, visitor_id, operator_id, date);
			if (last_id > 0)
				clear_records(shop_id, visitor_id, operator_id, last_id);

		} while (query.next());
	}


	return true;
}


uint_t CChatHistory::get_page_num(const uint_t visitor_id, const uint_t operator_id, const uint_t id, uint_t &page)
{
	// section: calc match start page
	QString conditions("hash = :hash AND \
((from_id = :visitor_id1 AND to_id = :operator_id1 AND direction = 0) OR \
(from_id = :operator_id2 AND to_id = :visitor_id2 AND direction = 1)) AND visible = 1 AND \
id >= :latest_id");

	QString sql("SELECT COUNT(id) FROM chat_log WHERE " + conditions);

	QSqlQuery query = QueryStart();
	query.prepare(sql);

	query.bindValue(":hash", GetShopHash());
	query.bindValue(":visitor_id1", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id1", static_cast<unsigned int>(operator_id));
	query.bindValue(":visitor_id2", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id2", static_cast<unsigned int>(operator_id));
	query.bindValue(":latest_id", static_cast<unsigned int>(id));

	// execute query
	QueryExecute(query);

	// return 'empty' if SQL failed
	const QSqlError error = query.lastError();
	if (error.type() != QSqlError::NoError)
		return false;

	long records_count = 0;
	if (query.first())
		records_count = query.value(0).toInt();

	if (records_count < 0)
		return false;

 	int pages_count = records_count / CHAT_LOG_MESSAGES_PER_PAGE;
	const bool equal = (records_count % CHAT_LOG_MESSAGES_PER_PAGE == 0) ? true : false;
	page = ((equal) ? pages_count-1 : pages_count);

	return true;
}

long CChatHistory::FindLatestId(const uint_t visitor_id, const uint_t operator_id, const QString &SearchString, uint_t &page)
{
	if (SearchString.length() <= 0)
		return -1;

	long first_id = -1;
	const QString search_pattern(SearchString.trimmed());

	QString conditions("hash = :hash AND \
((from_id = :visitor_id1 AND to_id = :operator_id1 AND direction = 0) OR \
(from_id = :operator_id2 AND to_id = :visitor_id2 AND direction = 1)) AND visible = 1 AND \
message LIKE(:search_pattern)");

	QString sql("SELECT id FROM chat_log WHERE " + conditions + " ORDER BY id DESC LIMIT 1");

	// assign SQL query
	QSqlQuery query = QueryStart();
	query.prepare(sql);

	query.bindValue(":hash", GetShopHash());
	query.bindValue(":visitor_id1", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id1", static_cast<unsigned int>(operator_id));
	query.bindValue(":visitor_id2", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id2", static_cast<unsigned int>(operator_id));
	query.bindValue(":search_pattern", "%" + search_pattern + "%");

	// execute query
	QueryExecute(query);

	if (query.first())
		first_id = query.value(0).toInt();

	if (first_id < 0)
		return -1;

	// section: calc match start page
	if (!get_page_num(visitor_id, operator_id, first_id, page))
		return -1;

	return first_id;
}

long CChatHistory::FindNextId(const uint_t visitor_id, const uint_t operator_id, const QString &SearchString, const uint_t start_id, uint_t &page)
{
	if (SearchString.length() <= 0)
		return -1;

	long match_id = -1;
	const QString search_pattern(SearchString.trimmed());

	QString conditions("hash = :hash AND \
((from_id = :visitor_id1 AND to_id = :operator_id1 AND direction = 0) OR \
(from_id = :operator_id2 AND to_id = :visitor_id2 AND direction = 1)) AND visible = 1 AND \
message LIKE(:search_pattern) AND id < :start_id");

	QString sql("SELECT id FROM chat_log WHERE " + conditions + " ORDER BY id DESC LIMIT 1");

	// assign SQL query
	QSqlQuery query = QueryStart();
	query.prepare(sql);

	query.bindValue(":hash", GetShopHash());
	query.bindValue(":visitor_id1", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id1", static_cast<unsigned int>(operator_id));
	query.bindValue(":visitor_id2", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id2", static_cast<unsigned int>(operator_id));
	query.bindValue(":search_pattern", "%" + search_pattern + "%");
	query.bindValue(":start_id", static_cast<unsigned int>(start_id));

	// execute query
	QueryExecute(query);

	if (query.first())
		match_id = query.value(0).toInt();

	if (match_id < 0)
		return -1;

	// section: calc match start page
	if (!get_page_num(visitor_id, operator_id, match_id, page))
		return -1;

	return match_id;
}

long CChatHistory::FindPreviousId(const uint_t visitor_id, const uint_t operator_id, const QString &SearchString, const uint_t start_id, uint_t &page)
{
	if (SearchString.length() <= 0)
		return -1;

	long match_id = -1;
	const QString search_pattern(SearchString.trimmed());

	QString conditions("hash = :hash AND \
((from_id = :visitor_id1 AND to_id = :operator_id1 AND direction = 0) OR \
(from_id = :operator_id2 AND to_id = :visitor_id2 AND direction = 1)) AND visible = 1 AND \
message LIKE(:search_pattern) AND id > :start_id");

	QString sql("SELECT id FROM chat_log WHERE " + conditions + " ORDER BY id ASC LIMIT 1");

	// assign SQL query
	QSqlQuery query = QueryStart();
	query.prepare(sql);

	query.bindValue(":hash", GetShopHash());
	query.bindValue(":visitor_id1", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id1", static_cast<unsigned int>(operator_id));
	query.bindValue(":visitor_id2", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id2", static_cast<unsigned int>(operator_id));
	query.bindValue(":search_pattern", "%" + search_pattern + "%");
	query.bindValue(":start_id", static_cast<unsigned int>(start_id));

	// execute query
	QueryExecute(query);

	if (query.first())
		match_id = query.value(0).toInt();

	if (match_id < 0)
		return -1;

	// section: calc match start page
	if (!get_page_num(visitor_id, operator_id, match_id, page))
		return -1;

	return match_id;
}