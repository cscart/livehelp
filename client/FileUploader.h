#pragma once

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include "Settings.h"
#include "common.h"

class CFileUploader : QObject
{
	Q_OBJECT

private:
	QString m_Filename;
	uint_t m_FileSize;
	cConnectionParams m_ConnectionParams;

public:
	CFileUploader(const cConnectionParams &params);
	~CFileUploader();

	bool UploadFile(const QString &username, const QString &password, const uint_t visitor_id, const QString &filename);


private slots:;
	void replyFinished(QNetworkReply*);
	void ProxyAuthRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);
	void AuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);

signals:;

	void OnError(const QString&);
	void OnFileUploaded(const QString&, const uint_t);

};
