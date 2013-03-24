#pragma once

#include <QDialog>
#include "ui_DialogOfflineMsgView.h"
#include "OfflineMessages.h"

class CDialogOfflineMsgView : public QDialog
{
	Q_OBJECT

private:
	Ui::DialogOfflineMsgView ui;

	COfflineMessageRecord record;
	bool is_reply;

public:
	CDialogOfflineMsgView(const COfflineMessageRecord &object);
	~CDialogOfflineMsgView();

	bool IsMarkUnread() const { return ui.cbMarkAsUnread->isChecked(); }

	bool IsReply() const { return is_reply; }
	QString GetReplyText() const { return ui.ptReply->toPlainText(); }

private slots:;
	void reply();

	void textChanged();
};


/*
class CDialogOfflineMsgReply : public QDialog
{
	Q_OBJECT

private:
	Ui::DialogOfflineMsgReply ui;

public:
	CDialogOfflineMsgReply();
	~CDialogOfflineMsgReply();
};
//*/