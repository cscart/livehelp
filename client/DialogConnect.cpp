#include "stdafx.h"
#include "DialogConnect.h"

cDialogConnect::cDialogConnect()
{
	ui.setupUi(this);

	const QIcon urlIcon(":/lh3/Resources/url.png");

	cSettings settings;
	connections = settings.GetConnectionParamsList();

	for (int i = 0; i < connections.size(); ++i)
	{
		const cConnectionParams &params = connections[i];
		ui.cbAliases->addItem(urlIcon, params.GetAlias());
	}
}


QString cDialogConnect::GetUsername() const
{
	return ui.leUsername->text();
}

QString cDialogConnect::GetPassword() const
{
	if (ui.lePassword->text().size() <= 0)
		return "";

	QCryptographicHash crypto(QCryptographicHash::Md5);
	crypto.addData(ui.lePassword->text().toUtf8());
	return QString(crypto.result().toHex());
}

cConnectionParams cDialogConnect::GetConnectionParams() const
{
	const int i = ui.cbAliases->currentIndex();
	if (i < 0 || i >= connections.size())
		return cConnectionParams();

	return connections[i];
}

