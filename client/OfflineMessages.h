#pragma once

//#include "singleton.h"
#include "DataBase.h"
#include "defines.h"
#include <list>

using std::list;

class COfflineMessageRecord
{
public:
	enum FeedbackState
	{
		FeedbackHash,
		FeedbackReply,
		FeedbackObsolete
	};

	enum Status
	{
		StatusNew,
		StatusPending,
		StatusReplied
	};

private:
	uint_t id;
	uint_t date;
	QString from_name;
	QString from_email;
	QString subject;
	QString message;
	QString reply;
	uint_t reply_date;
	uint_t operator_id;
	QString operator_name;
	Status status;
	bool unread;
	QString hash;

public:
	COfflineMessageRecord(const uint_t _id);

	uint_t GetId() const { return id; }

	void SetDate(const uint_t value) { date = value; }
	uint_t GetDate() const { return date; }

	void SetFromName(const QString &value) { from_name = value; }
	QString GetFromName() const { return from_name; }

	void SetFromEmail(const QString &value) { from_email = value; }
	QString GetFromEmail() const { return from_email; }

	void SetSubject(const QString &value) { subject = value; }
	QString GetSubject() const { return subject; }
	QString GetReplySubject() const { return QString("RE: ") + subject; }

	void SetMessage(const QString &value) { message = value; }
	QString GetMessage() const { return message; }

	void SetReply(const QString &value) { reply = value; }
	QString GetReply() const { return reply; }

	void SetReplyDate(const uint_t value) { reply_date = value; }
	uint_t GetReplyDate() const { return reply_date; }

	void SetOperatorId(const uint_t value) { operator_id = value; }
	uint_t GetOperatorId() const { return operator_id; }

	void SetOperatorName(const QString& value) { operator_name = value; }
	QString GetOperatorName() const { return operator_name; }

	void SetStatus(const Status value) { status = value; }
	Status GetStatus() const { return status; }
	QString GetStatusString() const;

	void SetUnread(const bool value) { unread = value; }
	bool IsUnread() const { return unread; }

	void SetHash(const QString &value) { hash = value; }
	QString GetHash() const { return hash; }

	QString GetDateString(const bool next_line=true) const;
	QString GetReplyDateString(const bool next_line=true) const;

	QIcon GetIcon() const;

	void GetShortParamsDOM(QDomElement &root, const FeedbackState state=FeedbackHash) const;

friend class COfflineMessages;
};

typedef list<COfflineMessageRecord> OfflineMessages;

class COfflineMessages : public CDataBase
{
private:
	static bool tables_initialized;

	virtual QSqlQuery init_tables(QSqlDatabase &database);
	virtual void SetTableInitialized() { tables_initialized = true; }

public:
/*
#ifdef __GNUC__
	// for g++ and gcc compilers
	COfflineMessages() :CSingleton<COfflineMessages>::CSingleton(*this) {}
#else
// _MSC_VER // for Microsoft C++ compilers
#pragma warning(disable : 4355)
	COfflineMessages() :CSingleton(*this) {}
#pragma warning(default : 4355)
#endif
//*/

	virtual bool IsTableInitialized() const { return tables_initialized; }

	OfflineMessages GetRecords();

	virtual void UpdateFromXML(const CNodeXML &root) {}
//	virtual void UpdateFromXML(const CNodeXML &root);


	void InsertUpdateRecord(const COfflineMessageRecord &record);
	void DeleteRecord(const uint_t id);
	bool GetRecord(const uint_t id, COfflineMessageRecord &record);
};
