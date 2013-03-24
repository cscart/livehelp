#include "stdafx.h"
#include "Settings.h"


cConnectionParams::cConnectionParams()
:use_auth(false)
{
	alias = "Shop";
	url = QUrl("http://");
}

QString cConnectionParams::GetShopHash() const
{
	const QString shop_url(url.toString().toLower());
	QCryptographicHash crypto(QCryptographicHash::Md5);
	crypto.addData(shop_url.toUtf8());
	return QString(crypto.result().toHex());
}


cProxyParams::cProxyParams()
:use_proxy(false), proxy_port(8080)
{}

QNetworkProxy cProxyParams::GetNetworkProxy() const
{
	const QString encrypted_password = decrypt(proxy_password);

	const QNetworkProxy::ProxyType type = (use_proxy) ? QNetworkProxy::HttpProxy : QNetworkProxy::NoProxy;
	QNetworkProxy proxy(type, proxy_url, proxy_port, proxy_username, encrypted_password);
	proxy.setCapabilities(QNetworkProxy::TunnelingCapability);// | QNetworkProxy::HostNameLookupCapability);
	return proxy;
}


ConnectionParamsList cSettings::GetConnectionParamsList()
{
	ConnectionParamsList list;

	QSettings &settings = get_settings_object();

	const int size = settings.beginReadArray("Settings/ConnectionParams");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex(i);
		cConnectionParams params;

		params.SetUrl(settings.value("url").toUrl());
		params.SetAlias(settings.value("alias").toString());

		params.SetUseAuth(settings.value("use_auth").toBool());
		params.SetAuthUsername(settings.value("auth_username").toString());
		
		const QString encrypted_password = settings.value("auth_password").toString();
		params.SetAuthPassword(decrypt(encrypted_password));

		list.push_back(params);
	}
	settings.endArray();

	return list;
}

void cSettings::SetConnectionParamsList(const ConnectionParamsList &list)
{
	QSettings &settings = get_settings_object();

	settings.remove("Settings/ConnectionParams");

	settings.beginWriteArray("Settings/ConnectionParams");
	for (int i = 0; i < list.size(); ++i)
	{
		const cConnectionParams &params = list.at(i);

		settings.setArrayIndex(i);

		settings.setValue("url", params.GetUrl());
		settings.setValue("alias", params.GetAlias());

		settings.setValue("use_auth", params.IsUseAuth());
		settings.setValue("auth_username", params.GetAuthUsername());

		const QString decrypted_password = params.GetAuthPassword();
		settings.setValue("auth_password", encrypt(decrypted_password));
	}
	settings.endArray();
}


cProxyParams cSettings::GetProxyParams()
{
	QSettings &settings = get_settings_object();

	cProxyParams params;
	params.SetUseProxy(settings.value("Settings/Proxy/use_proxy").toBool());
	params.SetProxyUrl(settings.value("Settings/Proxy/proxy_url").toString());
	params.SetProxyPort(settings.value("Settings/Proxy/proxy_port").toInt());
	params.SetProxyUsername(settings.value("Settings/Proxy/proxy_username").toString());

	const QString encrypted_password = settings.value("Settings/Proxy/proxy_password").toString();
	params.SetProxyPassword(decrypt(encrypted_password));

	return params;
}

void cSettings::SetProxyParams(const cProxyParams &params)
{
	QSettings &settings = get_settings_object();

	settings.setValue("Settings/Proxy/use_proxy", params.IsUseProxy());
	settings.setValue("Settings/Proxy/proxy_url", params.GetProxyUrl());
	settings.setValue("Settings/Proxy/proxy_port", params.GetProxyPort());
	settings.setValue("Settings/Proxy/proxy_username", params.GetProxyUsername());

	const QString decrypted_password = params.GetProxyPassword();
	settings.setValue("Settings/Proxy/proxy_password", encrypt(decrypted_password));
}


// Preset messages

QList<QString> cSettings::GetPresetMessages()
{
	QList<QString> list;
	QSettings &settings = get_settings_object();

	const int size = settings.beginReadArray("Settings/PresetMessages");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex(i);
		const QString text = settings.value("text").toString();
		list.push_back(text);
	}
	settings.endArray();

	return list;
}

void cSettings::SetPresetMessages(const QList<QString> &list)
{
	QSettings &settings = get_settings_object();

	settings.remove("Settings/PresetMessages");

	settings.beginWriteArray("Settings/PresetMessages");
	for (int i = 0; i < list.size(); ++i)
	{
		settings.setArrayIndex(i);
		settings.setValue("text", list[i]);
	}
	settings.endArray();
}





QColor cSettings::GetVisitorChatColor()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Chat/Colors/Visitor", QColor(Qt::red)).value<QColor>();
}
void cSettings::SetVisitorChatColor(const QColor &color)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Chat/Colors/Visitor", color);
}

QColor cSettings::GetOperatorChatColor()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Chat/Colors/Operator", QColor(Qt::blue)).value<QColor>();
}
void cSettings::SetOperatorChatColor(const QColor &color)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Chat/Colors/Operator", color);
}

QColor cSettings::GetServiceChatColor()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Chat/Colors/Service", QColor(Qt::darkGreen)).value<QColor>();
}
void cSettings::SetServiceChatColor(const QColor &color)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Chat/Colors/Service", color);
}

QFont cSettings::GetVisitorChatFont()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Chat/Fonts/Visitor", QFont()).value<QFont>();
}
void cSettings::SetVisitorChatFont(const QFont &font)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Chat/Fonts/Visitor", font);
}

QFont cSettings::GetOperatorChatFont()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Chat/Fonts/Operator", QFont()).value<QFont>();
}
void cSettings::SetOperatorChatFont(const QFont &font)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Chat/Fonts/Operator", font);
}

QFont cSettings::GetServiceChatFont()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Chat/Fonts/Service", QFont()).value<QFont>();
}
void cSettings::SetServiceChatFont(const QFont &font)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Chat/Fonts/Service", font);
}


//
// History
//
int cSettings::GetHistoryKeepingDays()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/History/KeepingDays", 7).toUInt();
}
void cSettings::SetHistoryKeepingDays(const int value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/History/KeepingDays", value);
}


//
// Raise
//
bool cSettings::IsRaiseNewVisitor()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Raise/NewVisitor", false).toBool();
}
void cSettings::SetRaiseNewVisitor(const bool value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Raise/NewVisitor", value);
}

bool cSettings::IsRaiseNewOfflineMessage()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Raise/OfflineMessage", false).toBool();
}
void cSettings::SetRaiseNewOfflineMessage(const bool value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Raise/OfflineMessage", value);
}

bool cSettings::IsRaiseNewMessage()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Raise/Message", false).toBool();
}
void cSettings::SetRaiseNewMessage(const bool value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Raise/Message", value);
}

bool cSettings::IsRaiseWaitingChat()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Raise/WaitingChat", false).toBool();
}
void cSettings::SetRaiseWaitingChat(const bool value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Raise/WaitingChat", value);
}

bool cSettings::IsHideOnProgrammClose()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Tray/MinimizeOnClose", false).toBool();
}
void cSettings::SetHideOnProgrammClose(const bool value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Tray/MinimizeOnClose", value);
}


//
// Sounds
//
bool cSettings::IsSoundNewVisitor()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Sounds/NewVisitor/Enabled", false).toBool();
}
void cSettings::SetSoundNewVisitor(const bool value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Sounds/NewVisitor/Enabled", value);
}
QString cSettings::GetSoundNewVisitorFile()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Sounds/NewVisitor/Filename", QString("")).toString();
}
void cSettings::SetSoundNewVisitorFile(const QString &value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Sounds/NewVisitor/Filename", value);
}

bool cSettings::IsSoundWaitingChat()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Sounds/WaitingChat/Enabled", false).toBool();
}
void cSettings::SetSoundWaitingChat(const bool value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Sounds/WaitingChat/Enabled", value);
}
QString cSettings::GetSoundWaitingChatFile()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Sounds/WaitingChat/Filename", QString("")).toString();
}
void cSettings::SetSoundWaitingChatFile(const QString &value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Sounds/WaitingChat/Filename", value);
}

bool cSettings::IsSoundNewReplyMessage()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Sounds/NewReplyMessage/Enabled", false).toBool();
}
void cSettings::SetSoundNewReplyMessage(const bool value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Sounds/NewReplyMessage/Enabled", value);
}
QString cSettings::GetSoundNewReplyMessageFile()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Sounds/NewReplyMessage/Filename", QString("")).toString();
}
void cSettings::SetSoundNewReplyMessageFile(const QString &value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Sounds/NewReplyMessage/Filename", value);
}

bool cSettings::IsSoundNewOfflineMessage()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Sounds/NewOfflineMessage/Enabled", false).toBool();
}
void cSettings::SetSoundNewOfflineMessage(const bool value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Sounds/NewOfflineMessage/Enabled", value);
}
QString cSettings::GetSoundNewOfflineMessageFile()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Sounds/NewOfflineMessage/Filename", QString("")).toString();
}
void cSettings::SetSoundNewOfflineMessageFile(const QString &value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Sounds/NewOfflineMessage/Filename", value);
}

bool cSettings::IsSoundAwayVisitor()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Sounds/AwayVisitor/Enabled", false).toBool();
}
void cSettings::SetSoundAwayVisitor(const bool value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Sounds/AwayVisitor/Enabled", value);
}
QString cSettings::GetSoundAwayVisitorFile()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Sounds/AwayVisitor/Filename", QString("")).toString();
}
void cSettings::SetSoundAwayVisitorFile(const QString &value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Sounds/AwayVisitor/Filename", value);
}


//
// Shortcuts
//

QKeySequence cSettings::GetShortcut(const ShortcutType shortcut)
{
	const QSettings &settings = get_settings_object();
	return settings.value(QString("Settings/Shortcuts/") + GetShortcutAlias(shortcut), QKeySequence()).value<QKeySequence>();
}

void cSettings::SetShortcut(const ShortcutType shortcut, const QKeySequence &value)
{
	QSettings &settings = get_settings_object();
	settings.setValue(QString("Settings/Shortcuts/") + GetShortcutAlias(shortcut), value);
}


//
// Languages
//
QString cSettings::GetLanguage()
{
	const QSettings &settings = get_settings_object();
	return settings.value("Settings/Language", QString("")).toString();
}
void cSettings::SetLanguage(const QString &value)
{
	QSettings &settings = get_settings_object();
	settings.setValue("Settings/Language", value);
}