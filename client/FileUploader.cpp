#include "stdafx.h"
#include "FileUploader.h"

CFileUploader::CFileUploader(const cConnectionParams &params)
:m_ConnectionParams(params)
{
}

CFileUploader::~CFileUploader()
{
}

bool CFileUploader::UploadFile(const QString &username, const QString &password, const uint_t visitor_id, const QString &filename)
{
	m_Filename = filename;
	QFile file(m_Filename);
	m_FileSize = file.size();

	QFileInfo fi(filename);
	QString short_filename = fi.fileName();

	if (!file.open(QIODevice::ReadOnly))
		return false;

	const cProxyParams proxyParams = cSettings::GetProxyParams();
	const QUrl url(m_ConnectionParams.GetUrl().toString() + QString("?dispatch=operator.file_upload&login=%1&password=%2&visitor_id=%3&filename=%4&no_session=1").arg(username, password, QString::number(visitor_id), short_filename));

	QNetworkAccessManager *manager = new QNetworkAccessManager;
	connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
	connect(manager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)), this, SLOT(ProxyAuthRequired(const QNetworkProxy&, QAuthenticator*)));
	connect(manager, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), this, SLOT(AuthenticationRequired(QNetworkReply*, QAuthenticator*)));

	if (proxyParams.IsUseProxy())
		manager->setProxy(proxyParams.GetNetworkProxy());

	QNetworkRequest request;
	request.setUrl(url);

	if (m_ConnectionParams.IsUseAuth())
	{
		request.setRawHeader("Authorization", "Basic " +
										 QByteArray(QString("%1:%2").arg(m_ConnectionParams.GetAuthUsername()).arg(m_ConnectionParams.GetAuthPassword()).toAscii()).toBase64());
	}

	request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data");
	request.setHeader(QNetworkRequest::ContentLengthHeader, QString::number(file.size()));

	QByteArray data = file.readAll();
	file.close();

	QNetworkReply *reply = manager->post(request, data);

	return true;
}

void CFileUploader::replyFinished(QNetworkReply *reply)
{
	reply->manager()->deleteLater();

	if (reply->error() == QNetworkReply::NoError)
	{
		// file uploaded successfully
		OnFileUploaded(m_Filename, m_FileSize);

	QByteArray arr = reply->readAll();
	QString xml_respond = QString::fromUtf8(arr, arr.length());

		return;
	}

	// error cause
	OnError(m_Filename);
}

void CFileUploader::AuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
}

void CFileUploader::ProxyAuthRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)
{
}
