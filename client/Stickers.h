#pragma once

//#include "singleton.h"
#include "DataBase.h"
#include "defines.h"
#include <list>

using std::list;

class CStickerRecord
{
public:
	enum FeedbackState
	{
		FeedbackHash,
		FeedbackNew,
		FeedbackObsolete
	};

private:
	uint_t id;
	uint_t operator_id;
	QString operator_name;
	uint_t date;
	QString title;
	QString message;
	bool unread;
	QString hash;

public:
	CStickerRecord(const uint_t _id);

	uint_t GetId() const { return id; }

	void SetOperatorId(const uint_t value) { operator_id = value; }
	uint_t GetOperatorId() const { return operator_id; }

	void SetOperatorName(const QString& value) { operator_name = value; }
	QString GetOperatorName() const { return operator_name; }

	void SetDate(const uint_t value) { date = value; }
	uint_t GetDate() const { return date; }

	void SetTitle(const QString &value) { title = value; }
	QString GetTitle() const { return title; }

	void SetMessage(const QString &value) { message = value; }
	QString GetMessage() const { return message; }

	void SetUnread(const bool value) { unread = value; }
	bool IsUnread() const { return unread; }
	QIcon GetIcon() const;

	void SetHash(const QString &value) { hash = value; }
	QString GetHash() const { return hash; }

	QString GetDateString(const bool next_line=true) const;

	void GetShortParamsDOM(QDomElement &root, const FeedbackState state=FeedbackHash) const;

friend class CStickers;
};

typedef list<CStickerRecord> StickersRecords;

class CStickers : public CDataBase//, public CSingleton<CStickers>
{
private:
	static bool tables_initialized;

	virtual QSqlQuery init_tables(QSqlDatabase &database);
	virtual void SetTableInitialized() { tables_initialized = true; }

public:
/*
#ifdef __GNUC__
	// for g++ and gcc compilers
	CStickers() :CSingleton<CStickers>::CSingleton(*this) {}
#else
// _MSC_VER // for Microsoft C++ compilers
#pragma warning(disable : 4355)
	CStickers() :CSingleton(*this) {}
#pragma warning(default : 4355)
#endif
//*/

	virtual bool IsTableInitialized() const { return tables_initialized; }

	StickersRecords GetRecords();

	virtual void UpdateFromXML(const CNodeXML &root) {}
//	virtual void UpdateFromXML(const CNodeXML &root);


	void InsertUpdateRecord(const CStickerRecord &record);
	void DeleteRecord(const uint_t id);
	bool GetRecord(const uint_t id, CStickerRecord &record);
};

//#define Stickers CStickers::getInstance()