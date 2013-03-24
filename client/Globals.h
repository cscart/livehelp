#pragma once

#include "common.h"
#include "Settings.h"

class cGlobals
{
private:
	static QString operator_name;
	static uint_t operator_id;

	static bool is_connected;
	static cConnectionParams m_ConnectionParams;
	static QString m_Login;
	static QString m_Password;

public:

	static QString GetAssignedOperatorName() { return operator_name; }
	static uint_t GetOperatorId() { return operator_id; }

//	static bool UpdateFromXML(const QString &xml_string);
	static bool UpdateFromXML(const CNodeXML &root);



	static void SetConnectionParams(const cConnectionParams &params, const QString &login, const QString &password);
	static void SetConnected() { is_connected = true; }
	static void SetDisconnected();

	static bool IsConnected() { return is_connected; }
	static const cConnectionParams& GetConnectionParams() { return m_ConnectionParams; }
	static const QString& GetLogin() { return m_Login; }
	static const QString& GetPassword() { return m_Password; }

};
