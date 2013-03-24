#pragma once

#include <QNetworkProxy>
#include <QSettings>
#include <QUrl>
#include <QColor>
#include <QKeySequence>
#include "common.h"

static QSettings& get_settings_object()
{
	static QSettings settings("settings.ini", QSettings::IniFormat);
	return settings;
}

class cConnectionParams
{
private:
	QString alias;
	QUrl url;

	bool use_auth;
	QString auth_username;
	QString auth_password;

public:

	cConnectionParams();

	const QString& GetAlias() const { return alias; }
	void SetAlias(const QString &s) { alias = s; }
	const QUrl& GetUrl() const { return url; }
	void SetUrl(const QUrl &_url) { url = _url; }

	bool IsUseAuth() const { return use_auth; }
	void SetUseAuth(const bool _val=false) { use_auth = _val; }
	const QString& GetAuthUsername() const { return auth_username; }
	void SetAuthUsername(const QString &s) { auth_username = s; }
	const QString& GetAuthPassword() const { return auth_password; }
	void SetAuthPassword(const QString &s) { auth_password = s; }

	QString GetShopHash() const;
};

class cProxyParams
{
private:
	bool use_proxy;
	QString proxy_url;
	int proxy_port;
	QString proxy_username;
	QString proxy_password;

public:
	cProxyParams();

	bool IsUseProxy() const { return use_proxy; }
	void SetUseProxy(const bool _val=false) { use_proxy = _val; }
	const QString& GetProxyUrl() const { return proxy_url; }
	void SetProxyUrl(const QString &_url) { proxy_url = _url; }
	int GetProxyPort() const { return proxy_port; }
	void SetProxyPort(const int _val) { proxy_port = _val; }
	const QString& GetProxyUsername() const { return proxy_username; }
	void SetProxyUsername(const QString &s) { proxy_username = s; }
	const QString& GetProxyPassword() const { return proxy_password; }
	void SetProxyPassword(const QString &s) { proxy_password = s; }

	QNetworkProxy GetNetworkProxy() const;

//	void ApplyGlobalProxySettings() const;
};

enum ShortcutType
{
	OpenConnectDialog,
	CloseConnection,
	ToggleVisitorsWindow,
	ToggleOfflineMessagesWindow,
	ToggleStickersWindow,
	ToggleVisitorsTrayWindow,
	OpenOptionsDialog,
	TileWindows,
	CascadeWindows,
	CloseActiveWindow,
	CloseAllWindows,
	ShortcutsCount
};

static QString GetShortcutAlias(const ShortcutType type)
{
	switch (type)
	{
		case OpenConnectDialog: return "OpenConnectDialog"; break;
		case CloseConnection: return "CloseConnection"; break;
		case ToggleVisitorsWindow: return "ToggleVisitorsWindow"; break;
		case ToggleOfflineMessagesWindow: return "ToggleOfflineMessagesWindow"; break;
		case ToggleStickersWindow: return "ToggleStickersWindow"; break;
		case ToggleVisitorsTrayWindow: return "ToggleVisitorsTrayWindow"; break;
		case OpenOptionsDialog: return "OpenOptionsDialog"; break;
		case TileWindows: return "TileWindows"; break;
		case CascadeWindows: return "CascadeWindows"; break;
		case CloseActiveWindow: return "CloseActiveWindow"; break;
		case CloseAllWindows: return "CloseAllWindows"; break;
	};

	return "";
}
/*
static QString GetShortcutDescription(const ShortcutType type)
{
	switch (type)
	{
		case OpenConnectDialog: return "Open connect dialog"; break;
		case CloseConnection: return "Close connection"; break;
		case ToggleVisitorsWindow: return "Toggle visitors window"; break;
		case ToggleOfflineMessagesWindow: return "Toggle offline messages window"; break;
		case ToggleStickersWindow: return "Toggle stickers window"; break;
		case ToggleVisitorsTrayWindow: return "Toggle visitors tray window"; break;
		case OpenOptionsDialog: return "Open options dialog"; break;
		case TileWindows: return "Tile windows"; break;
		case CascadeWindows: return "Cascade windows"; break;
		case CloseActiveWindow: return "Close active window"; break;
		case CloseAllWindows: return "Close all windows"; break;
	};

	return "";
}
//*/

typedef QList<cConnectionParams> ConnectionParamsList;

class cSettings
{
private:

public:
	static ConnectionParamsList GetConnectionParamsList();
	static void SetConnectionParamsList(const ConnectionParamsList &list);

	// Proxy params
	static cProxyParams GetProxyParams();
	static void SetProxyParams(const cProxyParams &params);

	// Chat params
  static QColor GetVisitorChatColor();
	static void SetVisitorChatColor(const QColor &color);
  static QColor GetOperatorChatColor();
	static void SetOperatorChatColor(const QColor &color);
  static QColor GetServiceChatColor();
	static void SetServiceChatColor(const QColor &color);

	static QFont GetVisitorChatFont();
	static void SetVisitorChatFont(const QFont &font);
	static QFont GetOperatorChatFont();
	static void SetOperatorChatFont(const QFont &font);
	static QFont GetServiceChatFont();
	static void SetServiceChatFont(const QFont &font);

	// History
	static int GetHistoryKeepingDays();
	static void SetHistoryKeepingDays(const int value);

	// Raise
	static bool IsRaiseNewVisitor();
	static void SetRaiseNewVisitor(const bool value);

	static bool IsRaiseNewOfflineMessage();
	static void SetRaiseNewOfflineMessage(const bool value);

	static bool IsRaiseNewMessage();
	static void SetRaiseNewMessage(const bool value);

	static bool IsRaiseWaitingChat();
	static void SetRaiseWaitingChat(const bool value);

	static bool IsHideOnProgrammClose();
	static void SetHideOnProgrammClose(const bool value);


	// Sounds
	static bool IsSoundNewVisitor();
	static void SetSoundNewVisitor(const bool value);
	static QString GetSoundNewVisitorFile();
	static void SetSoundNewVisitorFile(const QString &value);

	static bool IsSoundWaitingChat();
	static void SetSoundWaitingChat(const bool value);
	static QString GetSoundWaitingChatFile();
	static void SetSoundWaitingChatFile(const QString &value);

	static bool IsSoundNewReplyMessage();
	static void SetSoundNewReplyMessage(const bool value);
	static QString GetSoundNewReplyMessageFile();
	static void SetSoundNewReplyMessageFile(const QString &value);

	static bool IsSoundNewOfflineMessage();
	static void SetSoundNewOfflineMessage(const bool value);
	static QString GetSoundNewOfflineMessageFile();
	static void SetSoundNewOfflineMessageFile(const QString &value);

	static bool IsSoundAwayVisitor();
	static void SetSoundAwayVisitor(const bool value);
	static QString GetSoundAwayVisitorFile();
	static void SetSoundAwayVisitorFile(const QString &value);


	// Preset messages
	static QList<QString> GetPresetMessages();
	static void SetPresetMessages(const QList<QString> &list);


	// shortcuts
	static QKeySequence GetShortcut(const ShortcutType shortcut);
	static void SetShortcut(const ShortcutType shortcut, const QKeySequence &value);


	// Languages
	static QString GetLanguage();
	static void SetLanguage(const QString &value);
};
