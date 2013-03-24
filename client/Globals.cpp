#include "stdafx.h"
#include "Globals.h"

QString cGlobals::operator_name;
uint_t cGlobals::operator_id = 0;

bool cGlobals::is_connected = false;
cConnectionParams cGlobals::m_ConnectionParams;
QString cGlobals::m_Login;
QString cGlobals::m_Password;

void cGlobals::SetConnectionParams(const cConnectionParams &params, const QString &login, const QString &password)
{
	m_ConnectionParams = params;
	m_Login = login;
	m_Password = password;
}

void cGlobals::SetDisconnected()
{
	m_Login = m_Password = "";
	is_connected = false;
}

bool cGlobals::UpdateFromXML(const CNodeXML &root)
{
	QString value;
	bool result = true;

	if (root.GetValueByPath("Operator/Id", value))
		operator_id = value.toInt();
	else
		result = false;

	if (root.GetValueByPath("Operator/Name", value))
		operator_name = value;
	else
		result = false;

	return result;
}