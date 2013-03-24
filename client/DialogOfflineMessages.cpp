#include "stdafx.h"
#include "DialogOfflineMessages.h"

CDialogOfflineMsgView::CDialogOfflineMsgView(const COfflineMessageRecord &object)
:is_reply(false), record(object)
{
	ui.setupUi(this);

	ui.lSubject->setText(record.GetSubject());
	ui.lAuthor->setText(record.GetFromName());
	ui.lEmail->setText(record.GetFromEmail());
	ui.lDate->setText(record.GetDateString(false));
	ui.ptMessage->setPlainText(record.GetMessage());

	ui.lReplied->setText(record.GetStatusString());

	const QString respondent = (record.GetStatus() == COfflineMessageRecord::StatusReplied) ? record.GetOperatorName() : "-";
	const QString reply_date = (record.GetStatus() == COfflineMessageRecord::StatusReplied) ? record.GetReplyDateString(false) : "-";

	ui.lRespondent->setText(respondent);
	ui.lReplyDate->setText(reply_date);
	ui.ptReply->setPlainText(record.GetReply());

	ui.ptReply->setReadOnly(record.GetStatus() != COfflineMessageRecord::StatusNew);
}

CDialogOfflineMsgView::~CDialogOfflineMsgView()
{
}

void CDialogOfflineMsgView::reply()
{
//	if (QMessageBox::question(this, "Post Reply", "Are you sure?", "&Yes", "No", 0, 1, 0) != 0)
	if (DialogAreYouSure(this, tr("Post Reply", "MSG_POST_REPLY")) != 0)
		return;

	is_reply = true;
	accept();
}

void CDialogOfflineMsgView::textChanged()
{
	const QString text = ui.ptReply->toPlainText();
	const bool allow_reply = text.length() > 0 && record.GetStatus() == COfflineMessageRecord::StatusNew;
	ui.pbReply->setEnabled(allow_reply);
}

