#pragma once

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include "common.h"
#include "defines.h"

class CDataBase
{
#ifdef LOG_SQLITE_FILENAME
protected:
	void write_log(const QSqlQuery &query);
	void write_log(const QSqlError &error);
#endif

private:
	static QSqlDatabase db;
	static QString ShopHash;

	static int ref_count;

	virtual QSqlQuery init_tables(QSqlDatabase &database) = 0;

	virtual QString GetDatabaseName() const { return "reports.dat"; }

	virtual void SetTableInitialized() = 0;

	// copy operations restricted
	CDataBase(const CDataBase&);
	CDataBase& operator= (const CDataBase&);

public:
	CDataBase() { ++ref_count; }
	virtual ~CDataBase();

	bool Open();
	bool IsValid() const { return (db.isOpen() && /*ShopHash != "" &&*/ IsTableInitialized()) ? true : false; }

	virtual bool IsTableInitialized() const = 0;

	static void SetShopHash(const QString &hash) { ShopHash = hash; }
	static QString GetShopHash() { return ShopHash; }
	static void ClearShopHash() { ShopHash = ""; }


	bool DoQuery(const QString &s);
	QSqlQuery DoSelect(const QString &s);

	QSqlQuery QueryStart();
	bool QueryExecute(QSqlQuery &query);

	virtual void UpdateFromXML(const CNodeXML &root) = 0;
};

