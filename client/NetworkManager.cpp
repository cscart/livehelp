#include "stdafx.h"
#include "NetworkManager.h"

cNetworkManager::cNetworkManager()
:is_try_connect(false), is_test_connect(false), is_connected(false), waitDialog(NULL)
{
/*
	connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
	connect(&manager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)), this, SLOT(ProxyAuthRequired(const QNetworkProxy&, QAuthenticator*)));
	connect(&manager, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), this, SLOT(AuthenticationRequired(QNetworkReply*, QAuthenticator*)));
//*/

}

cNetworkManager::~cNetworkManager()
{

}

QString cNetworkManager::get_xml_request(const QDomElement &request_body)
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Root");
	doc.appendChild(root);

	// init auth values
	QDomElement auth = doc.createElement("Auth");
	root.appendChild(auth);

	QDomElement elem_username = doc.createElement("Username");
	elem_username.appendChild(doc.createTextNode(username));
	auth.appendChild(elem_username);

	QDomElement elem_password = doc.createElement("Password");
	elem_password.appendChild(doc.createTextNode(password));
	auth.appendChild(elem_password);

	if (!is_connected)
	{
		QDomElement elem_connect = doc.createElement("Connection");
		elem_connect.appendChild(doc.createTextNode("True"));
		auth.appendChild(elem_connect);
	}

	// Append feedback tracking data
//	OnComposeVisitorsShort(root);
	OnComposeRequest(root);

	// insert request body to the document
	root.appendChild(request_body);

	return "<?xml version='1.0' encoding='UTF-8' ?>\n" + doc.toString();
}

void cNetworkManager::send_request(const QDomElement &dom_request, const bool test_only)
{
	const QUrl url(connParams.GetUrl().toString() + ((test_only) ? "?dispatch=operator.test" : "?dispatch=operator.post") + "&no_session=1");
	QString xml_request = (test_only) ? "NULL" : get_xml_request(dom_request);

	urlencode(xml_request);

	QNetworkAccessManager *manager = new QNetworkAccessManager;
	connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
	connect(manager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)), this, SLOT(ProxyAuthRequired(const QNetworkProxy&, QAuthenticator*)));
	connect(manager, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), this, SLOT(AuthenticationRequired(QNetworkReply*, QAuthenticator*)));

	if (proxyParams.IsUseProxy())
		manager->setProxy(proxyParams.GetNetworkProxy());


	QNetworkRequest request;
	request.setUrl(url);

	request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows; U; Windows NT 5.1; ru; rv:1.9.0.6) Gecko/2009011913 Firefox/3.0.6 (.NET CLR 3.5.30729)");

	if (connParams.IsUseAuth())
	{
		const QString encrypted_password = connParams.GetAuthPassword();

		request.setRawHeader("Authorization", "Basic " +
										 QByteArray(QString("%1:%2").arg(connParams.GetAuthUsername()).arg(encrypted_password).toAscii()).toBase64());
	}

	QNetworkReply *reply = manager->post(request, "data=" + xml_request.toUtf8());

#ifdef LOG_NETWORK_REQUEST
	write_log(cNetworkManager::LOG_TYPE_REQUEST, xml_request);
#endif

	time.restart();
}

void cNetworkManager::send_all_data()
{
	QDomElement null_element;
	send_request(null_element);
}

void cNetworkManager::Connect(QWidget *parent, const QString &_username, const QString &_password, const cConnectionParams &_params)//, const QString &_url)
{
	if (is_try_connect || is_test_connect || IsConnected())
		return;

	proxyParams = cSettings::GetProxyParams();

	connParams = _params;

	username = _username;
	password = _password;

	open_wait_dialog(parent);

	is_try_connect = true;

	QDomElement null_element;
	send_request(null_element);
}

void cNetworkManager::Disconnect()
{
	stop_timer();
	if (is_connected)
	{
		is_connected = false;
		OnDisconnected();
	}
}

void cNetworkManager::TestConnection(QWidget *parent, const cConnectionParams &_params, const cProxyParams &proxy)
{
	if (is_try_connect || is_test_connect || IsConnected())
		return;

	proxyParams = proxy;

	connParams = _params;

	open_wait_dialog(parent);
	is_test_connect = true;

	QDomElement null_element;
	send_request(null_element, true);
}


void cNetworkManager::replyFinished(QNetworkReply *reply)
{
	reply->manager()->deleteLater();

	close_wait_dialog();

	const QNetworkReply::NetworkError error = reply->error();
	if (error != QNetworkReply::NoError || reply->size() == 0)
	{

#ifdef LOG_NETWORK_RESPOND_ERROR
		write_log(cNetworkManager::LOG_TYPE_ERROR, reply->errorString());
#endif

		if (is_try_connect)
		{
			OnTryConnectError(error, reply->errorString(), false);
			is_try_connect = false;
			return;
		}

		if (is_test_connect)
		{
			OnTestConnectError(error, reply->errorString(), false);
			is_test_connect = false;
			return;
		}

		Disconnect();
		OnConnectionError(error, reply->errorString(), false);
		return;
	}

	QByteArray arr = reply->readAll();
	QString xml_respond = QString::fromUtf8(arr, arr.length());

#ifdef LOG_NETWORK_RESPOND_PURE
	write_log(cNetworkManager::LOG_TYPE_PURE_RESPOND, xml_respond);
#endif

	const int start = xml_respond.indexOf("<?xml version='1.0'", 0);
	const int end = xml_respond.indexOf("</RootXmlRespond>", ((start > 0) ? start : 0));
	if (start >= 0 && end >= 0)
		xml_respond = xml_respond.mid(start, end-start+QString("</RootXmlRespond>").length());

#ifdef LOG_NETWORK_RESPOND_TRIM
	write_log(cNetworkManager::LOG_TYPE_TRIMMED_RESPOND, xml_respond);
#endif

	if (is_test_connect)
	{
		QString str_to_compare(xml_respond);
		str_to_compare.resize(ConnectionTestCheckSignature.length());
		if (str_to_compare == ConnectionTestCheckSignature)
			OnTestConnectSuccess();
		else
			OnTestConnectError(QNetworkReply::NoError, "", true);

		is_test_connect = false;
		return;
	}

	switch (check_auth_result(xml_respond))
	{
		default:
		case AuthFail:
			if (is_try_connect)
			{
				OnTryConnectError(QNetworkReply::NoError, "", true);
				is_try_connect = false;
				return;
			}

			Disconnect();
			OnConnectionError(QNetworkReply::NoError, "", true);
			return;
		break;

		case AuthOK:
			{
				if (is_try_connect)
				{
					is_connected = true;
					is_try_connect = false;
					OnConnected();
				}

				if (!is_connected)
					return;

				OnRespondReceivedXML(xml_respond);

				CNodeXML root;
				if (BuildXML(root, xml_respond))
					OnRespondReceivedXML(root);
			}
		break;

		case AuthActive: // TODO: Implement me
			if (is_try_connect)
			{
				OnConnectionActive();
				is_try_connect = false;
				return;
			}
		break;
	};

	bool is_opened_chat = false;
	OnCheckOpenedChat(is_opened_chat);
	const int default_track_interval = (is_opened_chat == true) ? iChatTimeInterval : iTrackTimeInterval;

	int track_interval = 250;
	const int ticks = time.restart();
	if (ticks < default_track_interval)
		track_interval = abs(default_track_interval - ticks);

	start_timer(track_interval);
}


void cNetworkManager::event_timer()
{
	stop_timer();
	if (!IsConnected())
		return;

	send_all_data();
}


cNetworkManager::AuthResult cNetworkManager::check_auth_result(const QString &string)
{
	QXmlStreamReader xml(string);
	while(!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();
		if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "AuthResult")
		{
			xml.readNext();
			if (xml.tokenType() != QXmlStreamReader::Characters)
				return AuthFail;

			if (xml.text() == "ACTIVE")
				return AuthActive;
			
			if (xml.text() == "OK")
				return AuthOK;

			break;
		}
	
		QString name = xml.name().toString();
		QString value = xml.text().toString();
	};

	return AuthFail;
}

void cNetworkManager::open_wait_dialog(QWidget *parent)
{
	waitDialog = new cDialogPleaseWait(parent);
	waitDialog->show();
}

void cNetworkManager::close_wait_dialog()
{
	if (waitDialog != NULL)
	{
		waitDialog->close();
		waitDialog = NULL;
	}
}

void cNetworkManager::start_timer(const int msec)
{
	timer.singleShot(msec, this, SLOT(event_timer()));
}

void cNetworkManager::urlencode(QString &value)
{
	value.replace("%", "%25");
	value.replace("&", "%26");
	value.replace("=", "%3D");
	value.replace("+", "%2B");
}

#ifdef LOG_NETWORK_FILENAME
void cNetworkManager::write_log(const LOG_TYPE type, const QString &_content)
{
	QString header;
	switch (type)
	{
		case LOG_TYPE_REQUEST: header += "REQUEST  "; break;
		case LOG_TYPE_PURE_RESPOND: header += "RESPOND PURE  "; break;
		case LOG_TYPE_TRIMMED_RESPOND: header += "RESPOND TRIM  "; break;
		case LOG_TYPE_ERROR: header += "ERROR  "; break;
	};

	WriteSeparateLog(LOG_NETWORK_FILENAME, _content, header);
}
#endif

void cNetworkManager::AuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
}

void cNetworkManager::ProxyAuthRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)
{
}

