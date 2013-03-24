#pragma once

#include <QtXml>
#include <QIcon>
#include <QApplication>
#include "common.h"
#include "ChatHistory.h"
#include "EventLog.h"
#include "list"

using std::list;

class CNote
{
public:
	enum Status
	{
		StatusHash,
		StatusNew,
		StatusObsolete
	};

private:
	uint_t id;
	uint_t operator_id;
	QString operator_name;
	uint_t date;
	QString message;

	friend bool operator< (const CNote &lhs, const CNote &rhs);

public:
	CNote(const uint_t value);
	CNote(const CNodeXML &root);

//	bool operator= (const CNote &rhs) { return id == rhs.id; }

	uint_t GetId() const { return id; }

	uint_t GetOperatorId() const { return operator_id; }
	void SetOperatorId(const uint_t value) { operator_id = value; }

	QString GetOperatorName() const { return operator_name; }
	void SetOperatorName(const QString &value) { operator_name = value; }

	void SetDate(const uint_t value) { date = value; }
	uint_t GetDate() const { return date; }

	void SetMessage(const QString &value) { message = value; }
	QString GetMessage() const { return message; }

	QString GetDateString(const bool next_line=true) const;

	void UpdateFromXML(const CNodeXML &root);
	void GetShortParamsDOM(QDomElement &root, const uint_t visitor_id, const Status status) const;
};

//static bool operator< (const CNote &lhs, const CNote &rhs) { return lhs.id < rhs.id; }

typedef list<CNote> NotesArray;



class CUploadedFile
{
public:
	enum Status
	{
		StatusHash,
		StatusPost,
		StatusObsolete
	};

	enum UploadState
	{
		UploadStateUnknown,
		UploadStateUploading,
		UploadStateUploaded,
		UploadStateError
	};

private:
	uint_t id;
	uint_t date;
	QString filename;
	uint_t size;
	uint_t operator_id;
	QString operator_name;
	UploadState state;

	friend bool operator< (const CNote &lhs, const CNote &rhs);

public:
	CUploadedFile(const uint_t value);
	CUploadedFile(const CNodeXML &root);

	uint_t GetId() const { return id; }

	void SetDate(const uint_t value) { date = value; }
	uint_t GetDate() const { return date; }

	void SetFilename(const QString &value) { filename = value; }
	QString GetFilename() const { return filename; }

	void SetSize(const uint_t value) { size = value; }
	uint_t GetSize() const { return size; }
	QString GetSizeString() const;

	uint_t GetOperatorId() const { return operator_id; }
	void SetOperatorId(const uint_t value) { operator_id = value; }

	QString GetOperatorName() const { return operator_name; }
	void SetOperatorName(const QString &value) { operator_name = value; }

	UploadState GetUploadState() const { return state; }
	void SetUploadState(const UploadState value) { state = value; }

	QString GetDateString(const bool next_line=true) const;

	void UpdateFromXML(const CNodeXML &root);
	void GetShortParamsDOM(QDomElement &root, const uint_t visitor_id, const Status status) const;
};

typedef list<CUploadedFile> UploadedFileArray;


class cVisitor : public QObject
{
	Q_OBJECT

private:
	struct FillHelper
	{
		QString key;
		QString &ref;
	};
	struct MatchHelper
	{
		QString key;
		QString value;
	};

public:
	enum Status
	{
		Idle = 0,
		WaitingChat = 1,
		Chatting = 2,
		ChatRequestSent = 3,
		CharRequestDeclined = 4,
		StatusCount = 5
	};

private:
	uint_t id;
//	QString name;
	QString nameByOperator;
	QString nameByVisitor;
	Status status;
	uint_t operator_id;

	uint_t ip;
	QString ip_str;
	QString operator_name;
	QString os;
	QString referrer;
	QString url;
	QString title;
	QString browser;
	QString country;

	NotesArray notes;
	UploadedFileArray files;

	QString hash;

	bool is_obsolete;

public: // TODO: move this 2 functions in protected area
	static QIcon GetNoUserIcon() { return QIcon(":/statuses/Resources/no_user.png"); }
	static QString GetNoUserDesrc() { return QApplication::trUtf8("Logged off", "MSG_VISITOR_LOGGED_OFF"); }

public:
	cVisitor();
	cVisitor(const cVisitor &rhs);
	cVisitor(const uint_t _id);
	~cVisitor();

	cVisitor& operator= (const cVisitor &rhs);

	uint_t GetId() const { return id; }
//	const QString& GetName() const { return name; }
//	const QString GetName() const { return GetCombinedName(); }
	const QString& GetNameByOperator() const { return nameByOperator; }
	const QString& GetNameByVisitor() const { return nameByVisitor; }
	QString GetCombinedName() const;
	QString GetCombinedChatName() const;
	Status GetStatus() const { return status; }
	uint_t GetOperatorId() const { return operator_id; }
	uint_t GetIpULong() const { return ip; }
	const QString& GetIpStr() const { return ip_str; }
	const QString& GetOperatorName() const { return operator_name; }
	const QString& GetOs() const { return os; }
	QIcon GetOsIcon() const;
	const QString& GetReferrer() const { return referrer; }
	const QString& GetUrl() const { return url; }
	const QString& GetTitle() const { return title; }
	const QString& GetBrowser() const { return browser; }
	QIcon GetBrowserIcon() const;
	const QString& GetCountry() const { return country; }
	QString GetCountryDesc() const { return ((country == "") ? QApplication::trUtf8("Unknown", "MSG_VISITOR_COUNTRY_UNKNOWN") : country); }
	QIcon GetCountryIcon() const;
	const QString& GetHash() const { return hash; }

	bool IsObsolete() const { return is_obsolete; }
	void SetObsolete(const bool val=false) { is_obsolete = val; }

	QIcon GetStatusIcon() const;
	QString GetStatusDescr() const;

	const NotesArray& GetNotes() const { return notes; }
	const UploadedFileArray& GetUploadedFiles() const { return files; }

	void Rename(const QString &new_name);


//	bool SetParamFromXML(QXmlStreamReader &xml);
	bool UpdateFromXML(const CNodeXML &root);

	void GetShortParamsDOM(QDomElement &root) const;

private slots:;
	

signals:;
	void OnStatusChanged(cVisitor::Status);
	void OnNameChanged(const QString&);

};
