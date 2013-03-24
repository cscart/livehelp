#pragma once

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtXml>
#include <QAuthenticator>

#include "DialogPleaseWait.h"
#include "Settings.h"
#include "defines.h"
#include "common.h"

const QString ConnectionTestCheckSignature("OK-LIVEHELP");
const int iTrackTimeInterval = 5000; // milliseconds
const int iChatTimeInterval = 2000; // milliseconds

class cNetworkManager : public QObject
{
	Q_OBJECT

#ifdef LOG_NETWORK_FILENAME
private:
	enum LOG_TYPE {LOG_TYPE_REQUEST, LOG_TYPE_PURE_RESPOND, LOG_TYPE_TRIMMED_RESPOND, LOG_TYPE_ERROR};

	void write_log(const LOG_TYPE type, const QString &_content);
#endif

private:
	enum AuthResult
	{
		AuthOK = 1,
		AuthFail,
		AuthActive
	};

	enum Asd
	{
		Asd12
	};

private:
	QString username;
	QString password;

	bool is_try_connect;
	bool is_test_connect;
	bool is_connected;

//	QNetworkAccessManager manager;

	cDialogPleaseWait *waitDialog;

	cConnectionParams connParams;
	cProxyParams proxyParams;

	QTimer timer;
	QTime time;

	QString get_xml_request(const QDomElement &request_body);
	void send_request(const QDomElement &dom_request, const bool test_only=false);
	AuthResult check_auth_result(const QString &string);

	void send_all_data();

	void open_wait_dialog(QWidget *parent);
	void close_wait_dialog();

	void start_timer(const int msec=iTrackTimeInterval);
	void stop_timer() { timer.stop(); }

	void urlencode(QString &value);

public:
	cNetworkManager();
	~cNetworkManager();

	void Connect(QWidget *parent, const QString &_username, const QString &_password, const cConnectionParams &_params);//, const QString &_url);
	void Disconnect();

	bool IsConnected() const { return is_connected; }

	void TestConnection(QWidget *parent, const cConnectionParams &_params, const cProxyParams &proxy);


private slots:;
	void replyFinished(QNetworkReply*);

	void event_timer();

	void ProxyAuthRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);
	void AuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);

signals:;

	void OnConnected();
	void OnDisconnected();
	void OnConnectionActive();
	void OnConnectionError(QNetworkReply::NetworkError, const QString&, bool);
	void OnTryConnectError(QNetworkReply::NetworkError, const QString&, bool);
	void OnTestConnectError(QNetworkReply::NetworkError, const QString&, bool);
	void OnTestConnectSuccess();

	void OnRespondReceivedXML(const QString&);
	void OnRespondReceivedXML(const CNodeXML&);

//	void OnComposeVisitorsShort(QDomElement&);

	void OnComposeRequest(QDomElement&);

	void OnCheckOpenedChat(bool&);

};

