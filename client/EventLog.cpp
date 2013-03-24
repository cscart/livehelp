#include "stdafx.h"
#include "EventLog.h"


QString CEventLogRecord::GetEventDesc(const Event event_id)
{
	switch (event_id)
	{
	case ChatStarted: return QApplication::trUtf8("Chat Started", "LBL_EVENT_CHAT_STARTED");
		case OperatorJoined: return QApplication::trUtf8("Operator Joined", "LBL_EVENT_OPERATOR_JOINED");
		case InvitationSent: return QApplication::trUtf8("Invitation Sent", "LBL_EVENT_INVITATION_SENT");
		case InvitationAccepted: return QApplication::trUtf8("Invitation Accepted", "LBL_EVENT_INVITATION_ACCEPTED");
		case InvitationDeclined: return QApplication::trUtf8("Invitation Declined", "LBL_EVENT_INVITATION_DECLINED");
		case PageChanged: return QApplication::trUtf8("Page Changed", "LBL_EVENT_PAGE_CHANGED");
	};

	return QApplication::trUtf8("Unknown", "LBL_EVENT_UNKNOWN");
}

QString CEventLogRecord::GetDateString() const
{
	QDateTime date_time;
	date_time.setTime_t(date);
	return date_time.toString("MMMM d, yyyy\nh:mm:ss ap");
}


bool CEventLog::tables_initialized = false;


QSqlQuery CEventLog::init_tables(QSqlDatabase &database)
{
	QSqlQuery query(database);

	query.exec("\
CREATE TABLE IF NOT EXISTS event_log (\
`hash` varchar(32) NOT NULL, \
`id` integer NOT NULL, \
`date` integer NOT NULL, \
`visitor_id` integer NOT NULL, \
`operator_id` integer NOT NULL, \
`operator_name` text, \
`event_id` integer(3) NOT NULL, \
`page_url` text, \
`page_title` text, \
PRIMARY KEY(`hash`, `id`))");


	if (query.lastError().type() != QSqlError::NoError)
		return query;

	query.exec("\
CREATE TABLE IF NOT EXISTS visitor_event_ids (\
`hash` varchar(32) NOT NULL default '', \
`visitor_id` integer NOT NULL default 0, \
`operator_id` integer NOT NULL default 0, \
`event_log_id` integer NOT NULL default 0, \
PRIMARY KEY(`hash`, `visitor_id`, `operator_id`)\
)");

	return query;
}

QString CEventLog::get_sql_conditions(QMap<QString, QVariant> &query_values, const uint_t visitor_id, const uint_t operator_id, const QDate &DateStart, const QDate &DateEnd, const QString &pattern, const list<int> &events)
{
	QSqlQuery query = QueryStart();
	query.prepare("SELECT event_log_id FROM visitor_event_ids WHERE hash = :hash AND visitor_id = :visitor_id AND operator_id = :operator_id");
	query.bindValue(":hash", GetShopHash());
	query.bindValue(":visitor_id", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id", static_cast<unsigned int>(operator_id));
	QueryExecute(query);

	const long id = (query.first()) ? query.value(0).toInt() : -1;

	// start date
	const QDateTime start_dt(DateStart);
	const unsigned long start_secs = start_dt.toTime_t();

	// end date
	const QDateTime end_dt(DateEnd);
	const unsigned long end_secs = end_dt.toTime_t() + 86400 - 1;

	// conditions query
	QString sql("hash = :hash AND visitor_id = :visitor_id AND date >= :start_date AND date <= :end_date");

	query_values.insert(":hash", GetShopHash());
	query_values.insert(":visitor_id", static_cast<unsigned int>(visitor_id));
	query_values.insert(":start_date", static_cast<unsigned int>(start_secs));
	query_values.insert(":end_date", static_cast<unsigned int>(end_secs));

	if (id > 0)
	{
		sql += " AND id > :id";
		query_values.insert(":id", static_cast<unsigned int>(id));
	}

	if (pattern != "")
	{
		sql += " AND (page_url LIKE(:search_patt1) OR page_title LIKE(:search_patt2))";
		query_values.insert(":search_patt1", "%" + pattern + "%");
		query_values.insert(":search_patt2", "%" + pattern + "%");
	}

	if (events.size() > 0)
	{
		QString id_list;
		bool is_first = true;
		for (list<int>::const_iterator it = events.begin(); it != events.end(); ++it)
		{
			if (!is_first)
				id_list += ",";
			else
				is_first = false;

			id_list += QString::number(*it);
		}

		sql += " AND event_id IN (" + id_list + ")";
	}

	return sql;
}


EventLogRecords CEventLog::GetRecords(const uint_t visitor_id, const uint_t operator_id, const QDate &DateStart, const QDate &DateEnd, const QString &pattern, const list<int> &events, const int page)
{
	QMap<QString, QVariant> query_values;

	const QString conditions(get_sql_conditions(query_values, visitor_id, operator_id, DateStart, DateEnd, pattern, events));
	QString sql("SELECT id, date, event_id, operator_name, page_url, page_title FROM event_log WHERE " + conditions);

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

	// traverse all rows
	EventLogRecords records;
	if (query.first())
	{
		do
		{
			// create and fill record object
			CEventLogRecord record(query.value(0).toInt());
			record.date = query.value(1).toInt();
			record.event_type = static_cast<CEventLogRecord::Event>(query.value(2).toInt());
			record.operator_name = query.value(3).toString();
			record.page_url = query.value(4).toString();
			record.page_title = query.value(5).toString();

			// push into the list
			records.push_back(record);
		} while (query.next());
	}

	return records;
}


uint_t CEventLog::GetRecordsCount(const uint_t visitor_id, const uint_t operator_id, const QDate &DateStart, const QDate &DateEnd, const QString &pattern, const list<int> &events)
{
	QMap<QString, QVariant> query_values;

	// retrieve condition string
	const QString conditions(get_sql_conditions(query_values, visitor_id, operator_id, DateStart, DateEnd, pattern, events));

	const QString sql("SELECT COUNT(id) FROM event_log WHERE " + conditions);

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

long CEventLog::GetLastEventId(const uint_t visitor_id)
{
	return get_last_event_id(GetShopHash(), visitor_id);
}

long CEventLog::get_last_event_id(const QString &shop_id, const uint_t visitor_id, const uint_t date)
{
	if (!IsValid())
		return -1;

	QSqlQuery query = QueryStart();
	query.prepare("SELECT MAX(id) FROM event_log WHERE hash = :hash AND visitor_id = :visitor_id" + ((date > 0) ? " AND date < " + QString::number(date) : ""));
	query.bindValue(":hash", shop_id);
	query.bindValue(":visitor_id", static_cast<unsigned int>(visitor_id));
	QueryExecute(query);

	// return 'empty' if SQL failed
	const QSqlError error = query.lastError();
	if (error.type() != QSqlError::NoError)
		return -1;

	if (query.first())
		return query.value(0).toInt();

	return 0;
}

void CEventLog::UpdateFromXML(const CNodeXML &root)
{
	if (!IsValid())
		return;

	CNodeXML events;
	if (root.GetNodeByPath("EventsLog", events) && events.GetNodesCount() > 0 && events.NodeStart())
	{
		do
		{
			CNodeXML _event = events.GetNode();

			bool failed = false;
			QString id, visitor_id, event_id, date, operator_id, operator_name, page_url, page_title;

			if (_event.HasAttribute("Id"))
				id = _event.GetAttributeString("Id");
			else
				failed = true;

			if (!_event.GetValueByPath("VisitorId", visitor_id))
				failed = true;

			if (!_event.GetValueByPath("Code", event_id))
				failed = true;

			if (!_event.GetValueByPath("Date", date))
				failed = true;

			// interrupt if error found
			if (failed)
				return;

			_event.GetValueByPath("OperatorId", operator_id);
			_event.GetValueByPath("OperatorName", operator_name);
			_event.GetValueByPath("PageURL", page_url);
			_event.GetValueByPath("PageTitle", page_title);

			// prepare and exec query
			QSqlQuery query = QueryStart();
/*
			query.prepare("INSERT INTO event_log \
(hash, id, date, visitor_id, operator_id, operator_name, event_id, page_title, page_url) VALUES \
(:hash, :id, :date, :visitor_id, :operator_id, :operator_name, :event_id, :page_title, :page_url)");
//*/

			query.prepare("REPLACE INTO event_log \
(hash, id, date, visitor_id, operator_id, operator_name, event_id, page_title, page_url) VALUES \
(:hash, :id, :date, :visitor_id, :operator_id, :operator_name, :event_id, :page_title, :page_url)");

			query.bindValue(":hash", GetShopHash());
			query.bindValue(":id", id);
			query.bindValue(":date", date);
			query.bindValue(":visitor_id", visitor_id);
			query.bindValue(":operator_id", operator_id);
			query.bindValue(":operator_name", operator_name);
			query.bindValue(":event_id", event_id);
			query.bindValue(":page_url", page_url);
			query.bindValue(":page_title", page_title);
			QueryExecute(query);

		} while (events.NodeNext());
	}
}

bool CEventLog::ClearRecords(const uint_t visitor_id, const uint_t operator_id)
{
	const long last_id = GetLastEventId(visitor_id);
	if (last_id <= 0)
		return false;

	return clear_record(GetShopHash(), visitor_id, operator_id, last_id);
}

bool CEventLog::clear_record(const QString &shop_id, const uint_t visitor_id, const uint_t operator_id, const uint_t last_id)
{
	if (shop_id == "")
		return false;

	QSqlQuery query = QueryStart();

	query.prepare("DELETE FROM event_log WHERE hash = :hash AND visitor_id = :visitor_id AND id < :last_id");
	query.bindValue(":hash", shop_id);
	query.bindValue(":visitor_id", static_cast<unsigned int>(visitor_id));
	query.bindValue(":last_id", static_cast<unsigned int>(last_id));
	QueryExecute(query);

	// prepare and exec query
	query.prepare("DELETE FROM visitor_event_ids WHERE hash = :hash AND visitor_id = :visitor_id AND operator_id = :operator_id");
	query.bindValue(":hash", shop_id);
	query.bindValue(":visitor_id", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id", static_cast<unsigned int>(operator_id));
	QueryExecute(query);

	// prepare and exec query
	query.prepare("INSERT INTO visitor_event_ids \
(hash, visitor_id, operator_id, event_log_id) VALUES \
(:hash, :visitor_id, :operator_id, :event_log_id)");
	query.bindValue(":hash", shop_id);
	query.bindValue(":visitor_id", static_cast<unsigned int>(visitor_id));
	query.bindValue(":operator_id", static_cast<unsigned int>(operator_id));
	query.bindValue(":event_log_id", static_cast<unsigned int>(last_id));
	QueryExecute(query);

	return true;
}

bool CEventLog::ClearAll(const uint_t date)
{
	QSqlQuery query = QueryStart();
	query.prepare("SELECT MAX(id) AS id, hash, visitor_id, operator_id FROM event_log WHERE operator_id > 0 GROUP BY hash, visitor_id, operator_id");
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

			const uint_t last_id = get_last_event_id(shop_id, visitor_id, date);
			if (last_id > 0)
				clear_record(shop_id, visitor_id, operator_id, last_id);

			int x = 0;
		} while (query.next());
	}

	return true;
}