#include "stdafx.h"
#include "common.h"

QString long2ip(unsigned long _ip)
{
	QString ip;

	const unsigned long p0 = _ip / 16777216;
	const unsigned long p1 = ((_ip -p0 * 16777216) / 65536);
	const unsigned long p2 = (_ip - p0 * 16777216 - p1 * 65536) / 256;
	const unsigned long p3 = _ip - p0 * 16777216 - p1 * 65536 - p2 * 256;

	ip += QString::number(p0, 10) + ".";
	ip += QString::number(p1, 10) + ".";
	ip += QString::number(p2, 10) + ".";
	ip += QString::number(p3, 10);

	return ip;
}

void WriteSeparateLog(const QString &_filename, const QString &_content, const QString &_header)
{
#ifdef LOG_CUMM_PATH
	if (QString(LOG_CUMM_PATH) != "")
		QDir::current().mkpath(LOG_CUMM_PATH);
#endif

	QString filename(_filename);
	if (QString(LOG_CUMM_PATH) != "")
		filename = QString(LOG_CUMM_PATH) + QDir::separator() + filename;

	QFile file(filename);
	if (file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
	{
		QString delim;
		delim.fill('=', 80);

		QString header;
		header += delim + "\n| " + _header;

		header += QDateTime::currentDateTime().toString("dd.MM.yyyy, hh:mm:ss") + "\n";

		QString delim_thin;
		delim_thin.fill('-', 80);
		header += delim_thin + "\n";
		file.write(header.toUtf8());

		const QString content = _content + "\n" + delim + "\n\n\n";

		file.write(content.toUtf8());
		file.close();
	}
}

int DialogAreYouSure(QWidget *parent, const QString &title)
{
	return QMessageBox::question(parent, title, parent->tr("Are you sure?", "MSG_ARE_YOU_SURE"),
		QString("&") + parent->tr("Yes", "MSG_YES"), parent->tr("No", "MSG_NO"), 0, 1, 0);
}
int DialogAreYouSure(QWidget *parent, const QString &title, const QString &message)
{
	return QMessageBox::question(parent, title, message, QString("&") + parent->tr("Yes", "MSG_YES"), parent->tr("No", "MSG_NO"), 0, 1, 0);
}

QString encrypt(const QString &str)
{
	QStdWString s = str.toStdWString();

	for (int i = 0; i < s.length(); ++i)
		s[i] = s[i] ^ encrypt_key;

	return QString(QString::fromStdWString(s));
}

QString decrypt(const QString &str)
{
	return encrypt(str);
}

//
// XML work
//

CNodeXML::CNodeXML()
{
	node_it = nodes.end();
}

bool CNodeXML::GetValueByPath(const QString &path, QString &value) const
{
	CNodeXML node;
	if (!GetNodeByPath(path, node))
		return false;

	value = node.GetValue();

	return true;
}

bool CNodeXML::GetNodeByPath(const QString &path, CNodeXML &node) const
{
	QStringList list = path.split("/");
	if (list.size() == 0)
		return false;

	const QString s = list.first();
	list.pop_front();

	NodesArray::const_iterator it = nodes.find(s);
	if (it == nodes.end())
		return false;

	if (list.size() == 0)
	{
		node = (*it).second;
		return true;
	}

	return (*it).second.GetNodeByPath(list.join("/"), node);
}

QString CNodeXML::GetAttributeString(const QString &name) const
{
	if (attributes.hasAttribute(name))
		return attributes.value(name).toString();
	
	return false;
}

bool CNodeXML::recursive_build(CNodeXML &result, QXmlStreamReader &xml)
{
	while(!xml.atEnd())
	{
		if (xml.isStartElement())
		{
			const QString name = xml.name().toString();

			result.attributes = xml.attributes();

			while (!xml.atEnd())
			{
				xml.readNext();
				if (xml.hasError())
				{
					write_log(xml);
					return false;
				}

				if (xml.isEndElement() && xml.name() == name)
					return true;

				if (xml.isWhitespace())
					continue;

				switch (xml.tokenType())
				{
					case QXmlStreamReader::StartElement:
					{
						CNodeXML sub_node;
						recursive_build(sub_node, xml);
						result.nodes.insert(std::pair<QString, CNodeXML> (xml.name().toString(), sub_node));
					}
					break;

					case QXmlStreamReader::Characters:
					{
						result.value = xml.text().toString();
					}
					break;

					default:
						continue;
				}
			}
		}

		xml.readNext();
		if (xml.hasError())
		{
			write_log(xml);
			return false;
		}
	}

	return true;
}

bool CNodeXML::NodeStart()
{
	if (nodes.size() == 0)
		return false;

	node_it = nodes.begin();
	return true;
}

bool CNodeXML::NodeNext()
{
	if (node_it == nodes.end())
		return false;

	++node_it;
	return node_it != nodes.end();
}

bool BuildXML(CNodeXML &result, const QString &xml_string)
{
	QXmlStreamReader xml(xml_string);

	CNodeXML obj;
	return obj.recursive_build(result, xml);
}

#ifdef LOG_XML_FILENAME
void CNodeXML::write_log(const QXmlStreamReader &xml)
{
	if (!xml.hasError())
		return;

	WriteSeparateLog(LOG_XML_FILENAME, xml.errorString());

	assert(false); // Parser got XML error
}
#endif