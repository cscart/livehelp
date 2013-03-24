#pragma once

#include "ui_DialogConnect.h"
#include "Settings.h"

class cDialogConnect : public QDialog
{
	Q_OBJECT

private:
	Ui::uiDialogConnect ui;

	ConnectionParamsList connections;

public:
	cDialogConnect();

	QString GetUsername() const;
	QString GetPassword() const;

	int GetConnectionsCount() const { return connections.size(); }

	cConnectionParams GetConnectionParams() const;
};
