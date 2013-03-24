#pragma once

#include "singleton.h"
#include "DataBase.h"
#include "defines.h"
#include "Globals.h"
#include <list>

using std::list;

class CEventLogRecord
{
public:
	enum Event
	{
		ChatStarted = 1,
		OperatorJoined,
		InvitationSent,
		InvitationAccepted,
		InvitationDeclined,
		PageChanged = 11
	};

private:
	uint_t id;
	uint_t date;
	Event event_type;
	QString operator_name;

	QString page_url;
	QString page_title;

public:
	CEventLogRecord(const uint_t _id) :id(_id) {}

	uint_t GetId() const { return id; }
	uint_t GetDate() const { return date; }
	Event GetEvent() const { return event_type; }
	QString GetOperatorName() const { return operator_name; }
	QString GetPageURL() const { return page_url; }
	QString GetPageTitle() const { return page_title; }

	QString GetEventDesc() const { return GetEventDesc(event_type); }
	static QString GetEventDesc(const Event event_id);
	QString GetDateString() const;

friend class CEventLog;
};

typedef list<CEventLogRecord> EventLogRecords;

class CEventLog : public CDataBase, public CSingleton<CEventLog>
{
private:
	static bool tables_initialized;

	virtual QSqlQuery init_tables(QSqlDatabase &database);
	QString get_sql_conditions(QMap<QString, QVariant> &query_values, const uint_t visitor_id, const uint_t operator_id, const QDate &DateStart, const QDate &DateEnd, const QString &pattern, const list<int> &events);

	virtual void SetTableInitialized() { tables_initialized = true; }
	bool clear_record(const QString &shop_id, const uint_t visitor_id, const uint_t operator_id, const uint_t last_id);
	long get_last_event_id(const QString &shop_id, const uint_t visitor_id, const uint_t date=0);

public:

#ifdef __GNUC__
// for g++ and gcc compilers
CEventLog() :CSingleton<CEventLog>::CSingleton(*this) {}
#else
// _MSC_VER // for Microsoft C++ compilers
#pragma warning(disable : 4355)
        CEventLog() :CSingleton(*this) {}
#pragma warning(default : 4355)
#endif

	virtual bool IsTableInitialized() const { return tables_initialized; }

	EventLogRecords GetRecords(const uint_t visitor_id, const uint_t operator_id, const QDate &DateStart, const QDate &DateEnd, const QString &pattern, const list<int> &events, const int page=0);
	uint_t GetRecordsCount(const uint_t visitor_id, const uint_t operator_id, const QDate &DateStart, const QDate &DateEnd, const QString &pattern, const list<int> &events);

	long GetLastEventId(const uint_t visitor_id);

	virtual void UpdateFromXML(const CNodeXML &root);

	bool ClearRecords(const uint_t visitor_id, const uint_t operator_id);
	bool ClearAll(const uint_t date);
};

#define EventLog CEventLog::getInstance()