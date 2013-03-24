#include "stdafx.h"
#include "DataBase.h"

QSqlDatabase CDataBase::db;
QString CDataBase::ShopHash;
int CDataBase::ref_count = 0;

CDataBase::~CDataBase()
{
	--ref_count;
	if (ref_count <= 0 && db.isOpen())
		db.close();
}

bool CDataBase::Open()
{
	if (db.isOpen())
	{
		if (IsTableInitialized())
			return true;

		const QSqlQuery query = init_tables(db);
		if (query.lastError().type() == QSqlError::NoError)
		{
			SetTableInitialized();
			return true;
		}

		write_log(query);
		return false;
	}

	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(GetDatabaseName());
	if (db.open())
	{
		const QSqlQuery query = init_tables(db);
		if (query.lastError().type() == QSqlError::NoError)
		{
			SetTableInitialized();
			return true;
		}

		write_log(query);
//		db.close();
		return false;
	}

	write_log(db.lastError());
	return false;
}

bool CDataBase::DoQuery(const QString &s)
{
	if (!IsValid())
		return false;

	QSqlQuery query(db);
	query.exec(s);

	write_log(query);

	return true;
}

QSqlQuery CDataBase::DoSelect(const QString &s)
{
	if (!IsValid())
		return QSqlQuery();

	QSqlQuery query(db);
	query.exec(s);

	write_log(query);

	return query;
}

QSqlQuery CDataBase::QueryStart()
{
	return QSqlQuery(db);
}

bool CDataBase::QueryExecute(QSqlQuery &query)
{
	const bool result = query.exec();
	write_log(query);
	return result;
}

#ifdef LOG_SQLITE_FILENAME
void CDataBase::write_log(const QSqlQuery &query)
{
	const QSqlError error = query.lastError();
	if (error.type() == QSqlError::NoError)
		return;

	WriteSeparateLog(LOG_SQLITE_FILENAME, query.lastQuery() + "\n\n" + error.text());

	assert(false); // Cause SQLITE error
}

void CDataBase::write_log(const QSqlError &error)
{
	if (error.type() == QSqlError::NoError)
		return;

	WriteSeparateLog(LOG_SQLITE_FILENAME, error.text());

	assert(false); // Cause SQLITE error
}
#endif