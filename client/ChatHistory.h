#pragma once

#include "singleton.h"
#include "DataBase.h"
#include "Visitor.h"
#include "defines.h"
#include "Globals.h"
#include <list>
#include <set>

using std::list;
using std::set;

class CChatHistoryRecord
{
public:
	enum Direction {VisitorToOperator, OperatorToVisitor};

private:
	uint_t id;
	uint_t date;
	Direction direction;
	QString text;
	bool matched;

public:
	CChatHistoryRecord(uint_t _id, uint_t _date, Direction _dir, const QString &_text);


	uint_t GetId() const { return id; }
	uint_t GetDate() const { return date; }
	Direction GetDirection() const { return direction; }
	QString GetString() const { return text; }

	QString GetDateString() const;

	bool IsMatched() const { return matched; }

friend class CChatHistory;
};

typedef list<CChatHistoryRecord> ChatHistoryRecords;

class CChatHistory : public CDataBase, public CSingleton<CChatHistory>
{
private:
	static bool tables_initialized;

	virtual QSqlQuery init_tables(QSqlDatabase &database);
	QString get_sql_conditions(QMap<QString, QVariant> &query_values, const uint_t visitor_id, const uint_t operator_id/*, const QString &SearchString*/) const;

	virtual void SetTableInitialized() { tables_initialized = true; }

	uint_t get_page_num(const uint_t visitor_id, const uint_t operator_id, const uint_t id, uint_t &page);

        bool clear_records(const QString &shop_id, const uint_t visitor_id, const uint_t operator_id, const uint_t last_id);
	long get_last_message_id(const QString &shop_id, const uint_t visitor_id, const uint_t operator_id, const uint_t date=0);

public:

#ifdef __GNUC__
// for g++ and gcc compilers
CChatHistory() :CSingleton<CChatHistory>::CSingleton(*this) {}
#else
// _MSC_VER // for Microsoft C++ compilers
#pragma warning(disable : 4355)
        CChatHistory() :CSingleton(*this) {}
#pragma warning(default : 4355)
#endif

	virtual bool IsTableInitialized() const { return tables_initialized; }

	ChatHistoryRecords GetRecords(const uint_t visitor_id, const uint_t operator_id, const QString &SearchString, const int page=0);
	uint_t GetRecordsCount(const uint_t visitor_id, const uint_t operator_id/*, const QString &SearchString*/);

	long GetLastMessageId(const uint_t visitor_id);

	virtual void UpdateFromXML(const CNodeXML &root);

	bool ClearRecords(const uint_t visitor_id);
	bool ClearAll(const uint_t date);

	long FindLatestId(const uint_t visitor_id, const uint_t operator_id, const QString &SearchString, uint_t &page);
	long FindNextId(const uint_t visitor_id, const uint_t operator_id, const QString &SearchString, const uint_t start_id, uint_t &page);
	long FindPreviousId(const uint_t visitor_id, const uint_t operator_id, const QString &SearchString, const uint_t start_id, uint_t &page);

};

#define ChatHistory CChatHistory::getInstance()
