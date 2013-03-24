#pragma once

#include <QFileDialog>
#include "ui_WidgetVisitorDoc.h"
#include "Visitor.h"
#include "Message.h"
#include "Settings.h"
#include "Globals.h"
#include "defines.h"
#include "FileUploader.h"
#include "Translator.h"

class cWidgetVisitorDoc : public QWidget, public ILanguageUI
{
	Q_OBJECT

private:
	enum DetailedInfoRow
	{
		Ip = 0,
		UserName,
		Status,
		Operator,
		Browser,
		Country,
		OS,
		Referrer,
		CurrentUrl,
		PageTitle
	};
	enum DetaildInfoColumn
	{
		Name,
		Value
	};
	enum NotesColumn
	{
		NoteColumnDate,
		NoteColumnId,
		NoteColumnOperatorId,
		NoteColumnAuthor,
		NoteColumnTest,
	};
	enum DownloadsColumn
	{
		DownloadsColumnDate,
		DownloadsColumnId,
		DownloadsColumnOperatorId,
		DownloadsColumnState,
		DownloadsColumnOperator,
		DownloadsColumnSize,
		DownloadsColumnFilename
	};


	#define CHAT_HISTORY_COLUMN_INDEX_DATE 0
	#define CHAT_HISTORY_COLUMN_INDEX_NAME 1
	#define CHAT_HISTORY_COLUMN_INDEX_TEXT 2

	#define EVENT_LOG_COLUMN_INDEX_DATE 0
	#define EVENT_LOG_COLUMN_INDEX_NAME 1
	#define EVENT_LOG_COLUMN_INDEX_TEXT 2

	typedef QList<QString> MessagesList;

private:
	Ui::uiDialogVisitorDoc ui;

	cVisitor visitor;

	MessagesList messages;

	NotesArray newNotes;
	NotesArray obsoleteNotes;

	UploadedFileArray postUploadedFiles;
	UploadedFileArray obsoleteUploadedFiles;

	// Chat History
	QString SearchString;
	uint_t ChatHistoryPage;
	uint_t ChatHistoryPagesCount;

	// Event Log
	QDate elDateStart;
	QDate elDateEnd;
	uint_t EventLogPage;
	uint_t EventLogPagesCount;
	QString elPattern;
	list<int> elEvents;

	uint_t found_message_id;

	void refresh();


	void update_detailed_info();
	void update_notes_page();
	void update_files_page();

	QString get_msg_invitation_sent() const { return tr("Invitation to chat has been sent to the visitor, reply is being awaited", "MSG_VISITOR_TEXT_INVITATION_SENT"); }
	QString get_msg_invitation_accepted() const { return tr("The visitor has accepted invitation", "MSG_VISITOR_TEXT_INVITATION_ACCEPTED"); }
	QString get_msg_invitation_declined() const { return tr("The visitor has declined the invitation", "MSG_VISITOR_TEXT_INVITATION_DECLINED"); }
	QString get_msg_chat_start() const { return tr("Chat session started", "MSG_VISITOR_TEXT_CHAT_START"); }
	QString get_msg_chat_end() const { return tr("Chat session has ended", "MSG_VISITOR_TEXT_CHAT_END"); }
	QString get_msg_chat_left() const { return tr("Visitor '[%VISITOR_OLD_NAME%]' has left the chat session", "MSG_VISITOR_TEXT_CHAT_LEFT"); }

	QString get_msg_visitor_new_name() const { return tr("Visitor got a new name '[%VISITOR_NEW_NAME%]'", "MSG_VISITOR_TEXT_VISITOR_NEW_NAME"); }

	void ClearReport(QTableWidget *widget);

	// Chat History
	bool chDoSearch();
	void chDisplayRecord(const CChatHistoryRecord &record);
//	void chClearReport();
	void chSetupPager(const uint_t PagesCount);


	// Event Log
	void elDoSearch();
	void elDisplayRecord(const CEventLogRecord &record);
//	void elClearReport();
	void elSetupPager(const uint_t PagesCount);


	// Notes
	void display_note_item(const CNote &note);

	// Files
	void display_file_item(const CUploadedFile &file);
	void download_get_states(bool &allow_delete, bool &allow_upload);

	// apply authority regarding to application state
	void apply_authority(const cVisitor &visitor);

public:
	cWidgetVisitorDoc(const cVisitor &_visitor);
	~cWidgetVisitorDoc();

	// get visitor's info
	uint_t GetVisitorId() const { return visitor.GetId(); }
	bool IsVisitorChatting() const { return (visitor.GetStatus() == cVisitor::Chatting) ? true : false; }


	void UpdateVisitor(const cVisitor &new_visitor);


// TODO:: Add visitor deleted action


	void GetMessageDOM(QDomElement &root);
	void GetNotesDOM(QDomElement &root);
	void GetFilesDOM(QDomElement &root);


	void DisplayMessage(const cMessage &msg);

	// override
	virtual void ApplyLanguageLabels() { ui.retranslateUi(this); }

private slots:;

	void SendMessage();

	void MessageTextChanged() { ui.pbSend->setEnabled((ui.leMessage->text().size() > 0) ? true : false); }

	// Preset messages
	void ShowPresetMessages();
	void PresetActionTriggered(QAction *action);

	// Chat History
	void chSearch();
	void chPageFirst();
	void chPagePrev();
	void chPageNext();
	void chPageLast();
	void chClearAll();

	void chNext();
	void chPrevious();
	void chLatest();


	// Event Log
	void elSearch();
	void elPageFirst();
	void elPagePrev();
	void elPageNext();
	void elPageLast();
	void elClearAll();


	// Notes
	void noteAdd();
	void noteDelete();
	void noteTextChanged();
	void noteItemSelectionChanged();


	// Downloads
	void downloadsDelete();
	void downloadsItemSelectionChanged();
	void downloadsUpload();
	void downloadsPostToChat();

	void downloadContextMenu(const QPoint &pt);
	void downloadContextPost(bool) { downloadsPostToChat(); }
	void downloadContextDelete(bool) { downloadsDelete(); }

signals:;



};
