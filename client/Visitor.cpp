#include "stdafx.h"
#include "Visitor.h"

CNote::CNote(const uint_t value)
:id(value), operator_id(0), date(0)
{
	date = QDateTime::currentDateTime().toTime_t();
}

CNote::CNote(const CNodeXML &root)
:id(0), operator_id(0), date(0)
{
	UpdateFromXML(root);
}

QString CNote::GetDateString(const bool next_line) const
{
	QDateTime date_time;
	date_time.setTime_t(date);
	return date_time.toString(QString("MMMM d, yyyy") + QString((next_line) ? "\n" : " ") + QString("h:mm:ss ap"));
}

void CNote::UpdateFromXML(const CNodeXML &root)
{
	QString str;

	// Id
	root.GetValueByPath("Id", str);
	id = str.toUInt();

	// OperatorId
	root.GetValueByPath("OperatorId", str);
	operator_id = str.toUInt();

	// OperatorName
	root.GetValueByPath("OperatorName", operator_name);

	// Date
	root.GetValueByPath("Date", str);
	date = str.toUInt();

	// Message
	root.GetValueByPath("Message", message);
}

void CNote::GetShortParamsDOM(QDomElement &root, const uint_t visitor_id, const Status status) const
{
	QDomDocument doc;
	QDomElement elem_note = doc.createElement("Note");
	root.appendChild(elem_note);

	switch (status)
	{
		case StatusNew:
		{
			elem_note.setAttribute("Type", "New");

			// visitor_id field
			QDomElement elem_visitor_id = doc.createElement("VisitorId");
			elem_visitor_id.appendChild(doc.createTextNode(QString::number(visitor_id)));
			elem_note.appendChild(elem_visitor_id);

			// message field
			QDomElement elem_message = doc.createElement("Message");
			elem_message.appendChild(doc.createTextNode(message));
			elem_note.appendChild(elem_message);
		}
		break;

		case StatusObsolete:
			elem_note.setAttribute("Id", QString::number(id));
			elem_note.setAttribute("Type", "Obsolete");
		break;
	};
}





CUploadedFile::CUploadedFile(const uint_t value)
:id(value), operator_id(0), date(0)
{
	date = QDateTime::currentDateTime().toTime_t();
}

CUploadedFile::CUploadedFile(const CNodeXML &root)
:id(0), operator_id(0), date(0)
{
	UpdateFromXML(root);
}

QString CUploadedFile::GetDateString(const bool next_line) const
{
	QDateTime date_time;
	date_time.setTime_t(date);
	return date_time.toString(QString("MMMM d, yyyy") + QString((next_line) ? "\n" : " ") + QString("h:mm:ss ap"));
}

void CUploadedFile::UpdateFromXML(const CNodeXML &root)
{
	QString str;

	// Id
	root.GetValueByPath("Id", str);
	id = str.toUInt();

	// OperatorId
	root.GetValueByPath("OperatorId", str);
	operator_id = str.toUInt();

	// OperatorName
	root.GetValueByPath("OperatorName", operator_name);

	// Date
	root.GetValueByPath("Date", str);
	date = str.toUInt();

	// Filename
	root.GetValueByPath("Filename", filename);

	// Size
	root.GetValueByPath("Size", str);
	size = str.toUInt();

	// State
	root.GetValueByPath("State", str);
	state = static_cast<UploadState>(str.toUInt());
}


void CUploadedFile::GetShortParamsDOM(QDomElement &root, const uint_t visitor_id, const Status status) const
{
	QDomDocument doc;
	QDomElement elem_note = doc.createElement("File");
	root.appendChild(elem_note);

	switch (status)
	{
/*
		case StatusNew:
		{
			elem_note.setAttribute("Type", "New");

			// visitor_id field
			QDomElement elem_visitor_id = doc.createElement("VisitorId");
			elem_visitor_id.appendChild(doc.createTextNode(QString::number(visitor_id)));
			elem_note.appendChild(elem_visitor_id);

			// message field
			QDomElement elem_message = doc.createElement("Message");
			elem_message.appendChild(doc.createTextNode(message));
			elem_note.appendChild(elem_message);
		}
		break;
//*/


		case StatusPost:
		{
			elem_note.setAttribute("Id", QString::number(id));
			elem_note.setAttribute("Type", "Post");
		}
		break;

		case StatusObsolete:
			elem_note.setAttribute("Id", QString::number(id));
			elem_note.setAttribute("Type", "Obsolete");
		break;
	};
}

QString CUploadedFile::GetSizeString() const
{
	switch (state)
	{
	case UploadStateUnknown: return QApplication::trUtf8("Unknown", "MSG_UNKNOWN");
		case UploadStateUploading: return QApplication::trUtf8("Uploading...", "MSG_UPLOADING");
		case UploadStateError: return QApplication::trUtf8("Error", "MSG_ERROR");
	};

	return QString::number(size) + QString(" ") + QApplication::trUtf8("Bytes", "MSG_BYTES");
}


cVisitor::cVisitor()
:id(0), status(Idle), operator_id(0), ip(0), is_obsolete(false)
{}

cVisitor::cVisitor(const cVisitor &rhs)
:id(0), status(Idle), operator_id(0), ip(0), is_obsolete(false)
{
	id = rhs.id;
//	name = rhs.name;
	nameByOperator = rhs.nameByOperator;
	nameByVisitor = rhs.nameByVisitor;
	status = rhs.status;
	operator_id = rhs.operator_id;

	ip = rhs.ip;
	ip_str = rhs.ip_str;
	operator_name = rhs.operator_name;
	os = rhs.os;
	referrer = rhs.referrer;
	url = rhs.url;
	title = rhs.title;
	browser = rhs.browser;
	country = rhs.country;

	is_obsolete = rhs.is_obsolete;

	notes = rhs.notes;
	files = rhs.files;

	hash = rhs.hash;
}

cVisitor::cVisitor(const uint_t _id)
:id(_id), status(Idle), operator_id(0), ip(0), is_obsolete(false)
{
}

cVisitor::~cVisitor()
{
}

cVisitor& cVisitor::operator= (const cVisitor &rhs)
{
	id = rhs.id;
//	name = rhs.name;
	nameByOperator = rhs.nameByOperator;
	nameByVisitor = rhs.nameByVisitor;
	status = rhs.status;
	operator_id = rhs.operator_id;

	ip = rhs.ip;
	ip_str = rhs.ip_str;
	operator_name = rhs.operator_name;
	os = rhs.os;
	referrer = rhs.referrer;
	url = rhs.url;
	title = rhs.title;
	browser = rhs.browser;
	country = rhs.country;

	is_obsolete = rhs.is_obsolete;

	notes = rhs.notes;
	files = rhs.files;

	hash = rhs.hash;

	return *this;
}


bool cVisitor::UpdateFromXML(const CNodeXML &root)
{
	QString str;

	// Id
	root.GetValueByPath("Id", str);
	id = str.toUInt();

	const FillHelper params[] = {/*"Name", name,*/ "NameByOperator", nameByOperator, "NameByVisitor", nameByVisitor, "Url", url, "Referrer", referrer, "Os", os,
		"OperatorName", operator_name, "Title", title, "Browser", browser, "Country", country, "Hash", hash};
	for (int i = 0; i < sizeof(params) / sizeof(params[0]); ++i)
	{
		root.GetValueByPath(params[i].key, params[i].ref);
	}

	// Status
	root.GetValueByPath("Status", str);
	switch (str.toUInt())
	{
		default:
		case 0: status = Idle; break;
		case 1: status = WaitingChat; break;
		case 2: status = Chatting; break;
		case 3: status = ChatRequestSent; break;
		case 4: status = CharRequestDeclined; break;
	}

	// OperatorId
	root.GetValueByPath("OperatorId", str);
	operator_id = str.toULong();

	// Ip
	root.GetValueByPath("Ip", str);
	ip = str.toULong();
	ip_str = long2ip(ip);


	// Notes
	CNodeXML notes_xml;
	if (root.GetNodeByPath("Notes", notes_xml) && notes_xml.GetNodesCount() > 0 && notes_xml.NodeStart())
	{
		do
		{
			CNodeXML note_xml = notes_xml.GetNode();

			CNote note(note_xml);
			notes.push_back(note);

		} while (notes_xml.NodeNext());
	}


	// Files
	CNodeXML files_xml;
	if (root.GetNodeByPath("Files", files_xml) && files_xml.GetNodesCount() > 0 && files_xml.NodeStart())
	{
		do
		{
			CNodeXML file_xml = files_xml.GetNode();

			CUploadedFile file(file_xml);
			files.push_back(file);

		} while (files_xml.NodeNext());
	}

	return true;
}

void cVisitor::GetShortParamsDOM(QDomElement &root) const
{
	QDomDocument doc;
	QDomElement elem_visitor = doc.createElement("Visitor");
	elem_visitor.setAttribute("Id", QString::number(id));
//	elem.appendChild(doc.createTextNode(hash));
	root.appendChild(elem_visitor);

	if (hash.size() > 0)
	{
		QDomElement elem_hash = doc.createElement("Hash");
		elem_hash.appendChild(doc.createTextNode(hash));
		elem_visitor.appendChild(elem_hash);
	}
	else
	{
		QDomElement elem_hash = doc.createElement("Name");
//		elem_hash.appendChild(doc.createTextNode(name));
		elem_hash.appendChild(doc.createTextNode(nameByOperator));
		elem_visitor.appendChild(elem_hash);
	}

	const int last_chat_id = ChatHistory->GetLastMessageId(id);
	if (last_chat_id >= 0)
	{
		QDomElement elem_hash = doc.createElement("LastChatMessageId");
		elem_hash.appendChild(doc.createTextNode(QString::number(last_chat_id)));
		elem_visitor.appendChild(elem_hash);
	}

	const int last_event_id = EventLog->GetLastEventId(id);
	if (last_event_id >= 0)
	{
		QDomElement elem_hash = doc.createElement("LastEventRecordId");
		elem_hash.appendChild(doc.createTextNode(QString::number(last_event_id)));
		elem_visitor.appendChild(elem_hash);
	}

}

QIcon cVisitor::GetStatusIcon() const
{
	if (IsObsolete())
		return GetNoUserIcon();

	QString path;

	switch (status)
	{
		case cVisitor::Idle: path = ":/statuses/Resources/visitor_idle.png"; break;
		case cVisitor::WaitingChat: path = ":/statuses/Resources/visitor_wait.png"; break;
		case cVisitor::Chatting: path = ":/statuses/Resources/visitor_chatting.png"; break;
		case cVisitor::ChatRequestSent: path = ":/statuses/Resources/visitor_sent.png"; break;
		case cVisitor::CharRequestDeclined: path = ":/statuses/Resources/visitor_declined.png"; break;
	};

	return QIcon(path);
}

QString cVisitor::GetStatusDescr() const
{
	if (IsObsolete())
		return GetNoUserDesrc();

	switch (status)
	{
		case cVisitor::WaitingChat: return QApplication::trUtf8("Waiting for chat", "MSG_VISITOR_WAITING_CHAT");
		case cVisitor::Chatting: return QApplication::trUtf8("Chatting", "MSG_VISITOR_CHATTING");
		case cVisitor::ChatRequestSent: return QApplication::trUtf8("Invitation sent", "MSG_VISITOR_INVITATION_SENT");
		case cVisitor::CharRequestDeclined: return QApplication::trUtf8("Invitation declined", "MSG_VISITOR_INVITATION_DECLINED");
	};

	return QApplication::trUtf8("Idle", "MSG_VISITOR_IDLE");
}

QIcon cVisitor::GetBrowserIcon() const
{
	MatchHelper params[] = {
		"Explorer", ":/browsers/Resources/explorer.png",
		"Firefox", ":/browsers/Resources/firefox.png",
		"Opera", ":/browsers/Resources/opera.png",
		"Safari", ":/browsers/Resources/safari.png",
		"Mozilla", ":/browsers/Resources/mozilla.png",
		"Netscape", ":/browsers/Resources/netscape.png"
	};

	for (int i = 0; i < sizeof(params) / sizeof(params[0]); ++i)
		if (browser.contains(params[i].key))
			return QIcon(params[i].value);

	return QIcon(":/browsers/Resources/unknown.png");
}

QIcon cVisitor::GetOsIcon() const
{
	MatchHelper params[] = {
		"Windows", ":/os/Resources/windows.png",
		"Linux", ":/os/Resources/linux.png",
		"Mac", ":/os/Resources/mac.png",
	};

	for (int i = 0; i < sizeof(params) / sizeof(params[0]); ++i)
		if (os.contains(params[i].key))
			return QIcon(params[i].value);

	return QIcon(":/os/Resources/unknown.png");
}

QIcon cVisitor::GetCountryIcon() const
{
	if (country == "")
		return QIcon(":/country/Resources/unknown.png");

	const QIcon ico(":/country/Resources/countries/" + country.toLower() + ".png");

	if (ico.isNull())
		return QIcon(":/country/Resources/unknown.png");

	return ico;
}

void cVisitor::Rename(const QString &new_name)
{
	hash = "";
//	name = new_name;
	nameByOperator = new_name;
}

QString cVisitor::GetCombinedName() const
{
	if (GetNameByOperator() == GetNameByVisitor())
		return GetNameByOperator();

	if (GetNameByOperator() == "")
		return GetNameByVisitor();

	if (GetNameByVisitor() == "")
		return GetNameByOperator();

	return GetNameByOperator() + QString(" (") + GetNameByVisitor() + QString(")");
}

QString cVisitor::GetCombinedChatName() const
{
	return GetCombinedName();

/*
	if (GetNameByOperator() == GetNameByVisitor())
		return GetNameByOperator();

	return GetNameByOperator() + QString(" (") + GetNameByVisitor() + QString(")");
//*/
}