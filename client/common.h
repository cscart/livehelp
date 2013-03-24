#pragma once

#include <QtXml>
#include <QXmlStreamAttributes>
#include <map>
#include <assert.h>
#include "defines.h"

using std::multimap;

typedef unsigned long int uint_t;

QString long2ip(unsigned long _ip);

void WriteSeparateLog(const QString &filename, const QString &_content, const QString &_header="");

int DialogAreYouSure(QWidget *parent, const QString &title);
int DialogAreYouSure(QWidget *parent, const QString &title, const QString &message);

static const int encrypt_key = 76; // ecryption key

QString encrypt(const QString &str);
QString decrypt(const QString &str);

//
// XML work
//
class CNodeXML
{
#ifdef LOG_XML_FILENAME
private:
	void write_log(const QXmlStreamReader &xml);
#endif

private:
	typedef multimap<QString, CNodeXML> NodesArray;

	NodesArray nodes;
	QString value;
	QXmlStreamAttributes attributes;

	NodesArray::iterator node_it;

	friend bool BuildXML(CNodeXML &result, const QString &xml_string);
	bool recursive_build(CNodeXML &result, QXmlStreamReader &xml);

public:
	CNodeXML();

	bool GetValueByPath(const QString &path, QString &value) const;
	bool GetNodeByPath(const QString &path, CNodeXML &node) const;

	bool HasAttribute(const QString &name) const { return attributes.hasAttribute(name); }
	QString GetAttributeString(const QString &name) const;
	int GetAttributesCount() const { return attributes.size(); }

	QString GetValue() const { return value; }

	bool NodeStart();
	bool NodeNext();
	CNodeXML GetNode() { return (*node_it).second; }
	size_t GetNodesCount() const { return nodes.size(); }

};

bool BuildXML(CNodeXML &result, const QString &xml_string);