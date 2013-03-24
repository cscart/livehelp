#include "stdafx.h"
#include "WidgetVisitorDoc.h"

cWidgetVisitorDoc::cWidgetVisitorDoc(const cVisitor &_visitor)
:visitor(_visitor), ChatHistoryPage(0), ChatHistoryPagesCount(0), found_message_id(-1)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	ui.deEventLogDateEnd->setDate(QDate::currentDate());

	// Notes
	ui.twNotes->header()->hideSection(NoteColumnId);
	ui.twNotes->header()->hideSection(NoteColumnOperatorId);

	// Downloads
	ui.twDownloads->header()->hideSection(DownloadsColumnId);
	ui.twDownloads->header()->hideSection(DownloadsColumnOperatorId);
	ui.twDownloads->header()->hideSection(DownloadsColumnState);

//	chSetupPager(0);
//	elSetupPager(0);

	chSearch();
	elSearch();

	// prepare events list
	CEventLogRecord::Event events[] = {CEventLogRecord::ChatStarted, CEventLogRecord::OperatorJoined, CEventLogRecord::InvitationSent,
																			CEventLogRecord::InvitationAccepted, CEventLogRecord::InvitationDeclined, CEventLogRecord::PageChanged};
	for (unsigned int i = 0; i < sizeof(events) / sizeof(events[0]); ++i)
	{
		QListWidgetItem *item = new QListWidgetItem(CEventLogRecord::GetEventDesc(events[i]));
		item->setData(Qt::UserRole, events[i]);
		ui.lwEventLogEvents->addItem(item);
	}

	ui.twDownloads->setContextMenuPolicy(Qt::CustomContextMenu);
//	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(downloadContextMenu(QPoint)));

	apply_authority(visitor);

	refresh();
}

cWidgetVisitorDoc::~cWidgetVisitorDoc()
{
}


void cWidgetVisitorDoc::refresh()
{
	// title
	const QString title = visitor.GetStatusDescr() + " - [" + visitor.GetCombinedName() + "]";
	setWindowTitle(title);

	// icon
	setWindowIcon(visitor.GetStatusIcon());

	// update all sections
	update_detailed_info();
	update_notes_page();
	update_files_page();
}


void cWidgetVisitorDoc::UpdateVisitor(const cVisitor &new_visitor)
{
	const cVisitor::Status old_status = visitor.GetStatus();
	const cVisitor::Status new_status = new_visitor.GetStatus();

	apply_authority(new_visitor);

	// display service messages
	QList<QString> service_msgs;
	if (old_status == cVisitor::ChatRequestSent && new_status == cVisitor::Chatting)
		service_msgs.push_back(get_msg_invitation_accepted()); // Invitation Sent ==> Chatting
	if (old_status == cVisitor::ChatRequestSent && (new_status == cVisitor::CharRequestDeclined || new_status == cVisitor::Idle))
		service_msgs.push_back(get_msg_invitation_declined()); // Invitation Sent ==> Invitation declined
	if (old_status != cVisitor::Chatting && new_status == cVisitor::Chatting)
		service_msgs.push_back(get_msg_chat_start()); // !Chatting ==> Chatting
	else if (old_status == cVisitor::Chatting && new_status != cVisitor::Chatting)
		service_msgs.push_back((new_status == cVisitor::Idle) ? get_msg_chat_left() : get_msg_chat_end()); // Chatting ==> (!Chatting || Idle)
//	else if (old_status != cVisitor::ChatRequestSent && new_status == cVisitor::ChatRequestSent)
//		service_msgs.push_back(get_msg_invitation_sent()); // !'Invitation Sent' ==> Invitation Sent

	if (visitor.GetNameByVisitor() != new_visitor.GetNameByVisitor())
		service_msgs.push_back(get_msg_visitor_new_name());

	for (size_t i = 0; i < service_msgs.size(); ++i)
	{
		QString text = service_msgs[i];
		text.replace("[%VISITOR_OLD_NAME%]", visitor.GetNameByVisitor(), Qt::CaseInsensitive);
		text.replace("[%VISITOR_NEW_NAME%]", new_visitor.GetNameByVisitor(), Qt::CaseInsensitive);
		cMessage message(0, cMessage::ChatEvent);
		message.SetText(text);
		DisplayMessage(message);
	}

	visitor = new_visitor;
	refresh();
}


void cWidgetVisitorDoc::apply_authority(const cVisitor &visitor)
{
	bool tabChatEnabled = true;

	if (visitor.GetOperatorId() > 0 && visitor.GetOperatorId() != cGlobals::GetOperatorId())
		tabChatEnabled = false;

	ui.tabChat->setEnabled(tabChatEnabled);
}


void cWidgetVisitorDoc::update_detailed_info()
{
	QTableWidgetItem *item = NULL;

	// IP address
	if (item = ui.twDetailedInfo->item(cWidgetVisitorDoc::Ip, cWidgetVisitorDoc::Value))
		item->setText(visitor.GetIpStr());

	// Username
	if (item = ui.twDetailedInfo->item(cWidgetVisitorDoc::UserName, cWidgetVisitorDoc::Value))
		item->setText(visitor.GetCombinedName());

	// Status
	if (item = ui.twDetailedInfo->item(cWidgetVisitorDoc::Status, cWidgetVisitorDoc::Value))
	{
		item->setIcon(visitor.GetStatusIcon());
		item->setText(visitor.GetStatusDescr());
	}

	// Operator
	if (item = ui.twDetailedInfo->item(cWidgetVisitorDoc::Operator, cWidgetVisitorDoc::Value))
		item->setText(visitor.GetOperatorName());

	// Browser
	if (item = ui.twDetailedInfo->item(cWidgetVisitorDoc::Browser, cWidgetVisitorDoc::Value))
	{
		item->setIcon(visitor.GetBrowserIcon());
		item->setText(visitor.GetBrowser());
	}

	// Country
	if (item = ui.twDetailedInfo->item(cWidgetVisitorDoc::Country, cWidgetVisitorDoc::Value))
	{
		item->setIcon(visitor.GetCountryIcon());
		item->setText(visitor.GetCountryDesc());
	}

	// OS
	if (item = ui.twDetailedInfo->item(cWidgetVisitorDoc::OS, cWidgetVisitorDoc::Value))
	{
		item->setIcon(visitor.GetOsIcon());
		item->setText(visitor.GetOs());
	}

	// Referrer
	if (item = ui.twDetailedInfo->item(cWidgetVisitorDoc::Referrer, cWidgetVisitorDoc::Value))
		item->setText(visitor.GetReferrer());

	// CurrentUrl
	if (item = ui.twDetailedInfo->item(cWidgetVisitorDoc::CurrentUrl, cWidgetVisitorDoc::Value))
		item->setText(visitor.GetUrl());

	// PageTitle
	if (item = ui.twDetailedInfo->item(cWidgetVisitorDoc::PageTitle, cWidgetVisitorDoc::Value))
		item->setText(visitor.GetTitle());

	// Stop chat button
//	ui.pbCloseChat->setEnabled((visitor.GetStatus() == cVisitor::Chatting) ? true : false);
}

void cWidgetVisitorDoc::update_notes_page()
{
	ui.twNotes->clear();

	const NotesArray &notes = visitor.GetNotes();

	for (NotesArray::const_iterator it = notes.begin(); it != notes.end(); ++it)
	{
		const CNote &note = *it;
		display_note_item(note);
	}
}

void cWidgetVisitorDoc::update_files_page()
{
	ui.twDownloads->clear();

	const UploadedFileArray &files = visitor.GetUploadedFiles();

	for (UploadedFileArray::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		const CUploadedFile &file = *it;
		display_file_item(file);
	}
}


void cWidgetVisitorDoc::GetMessageDOM(QDomElement &root)
{
	QDomDocument doc;

	for (MessagesList::const_iterator it = messages.begin(); it != messages.end(); ++it)
	{
		const QString &message = *it;

		QDomElement elem = doc.createElement("Message");
		elem.setAttribute("VisitorId", QString::number(visitor.GetId()));
		elem.appendChild(doc.createCDATASection(message));
		root.appendChild(elem);
	}

	messages.clear();
}

void cWidgetVisitorDoc::GetNotesDOM(QDomElement &root)
{
	for (NotesArray::const_iterator it = newNotes.begin(); it != newNotes.end(); ++it)
	{
		const CNote &note = *it;
		note.GetShortParamsDOM(root, visitor.GetId(), CNote::StatusNew);
	}

	for (NotesArray::const_iterator it = obsoleteNotes.begin(); it != obsoleteNotes.end(); ++it)
	{
		const CNote &note = *it;
		note.GetShortParamsDOM(root, visitor.GetId(), CNote::StatusObsolete);
	}

	obsoleteNotes.clear();
	newNotes.clear();
}

void cWidgetVisitorDoc::GetFilesDOM(QDomElement &root)
{
	for (UploadedFileArray::const_iterator it = obsoleteUploadedFiles.begin(); it != obsoleteUploadedFiles.end(); ++it)
	{
		const CUploadedFile &file = *it;
		file.GetShortParamsDOM(root, visitor.GetId(), CUploadedFile::StatusObsolete);
	}

	for (UploadedFileArray::const_iterator it = postUploadedFiles.begin(); it != postUploadedFiles.end(); ++it)
	{
		const CUploadedFile &file = *it;
		file.GetShortParamsDOM(root, visitor.GetId(), CUploadedFile::StatusPost);
	}

	obsoleteUploadedFiles.clear(); // TODO:: uncomment!!!
	postUploadedFiles.clear();
}

void cWidgetVisitorDoc::SendMessage()
{
	const QString text = ui.leMessage->text();
	if (text.length() <= 0)
		return;

	ui.leMessage->clear();
	messages.push_back(text);

	cMessage message(0, cMessage::OperatorToVisitor);
	message.SetText(text);
	DisplayMessage(message);

	// Post Event message: Invitation sent
	if (visitor.GetStatus() == cVisitor::Idle)
	{
		cMessage message(0, cMessage::ChatEvent);
		message.SetText(get_msg_invitation_sent());
		DisplayMessage(message);
	}
}

void cWidgetVisitorDoc::DisplayMessage(const cMessage &msg)
{
//	QString person_name((msg.GetDirection() == cMessage::VisitorToOperator) ? visitor.GetName() : visitor.GetOperatorName());
	QColor color(cSettings::GetOperatorChatColor());
	QFont font(cSettings::GetOperatorChatFont());
	QString person_name = "";

	switch (msg.GetDirection())
	{
		case cMessage::VisitorToOperator:
			color = cSettings::GetVisitorChatColor();
			font = cSettings::GetVisitorChatFont();
			person_name = visitor.GetCombinedChatName();
		break;

		case cMessage::OperatorToVisitor:
		{
			const QString operator_name(visitor.GetOperatorName());
			person_name = ((operator_name.size() <= 0) ? cGlobals::GetAssignedOperatorName() : operator_name);
		}
		break;

		default:
		case cMessage::ChatEvent:
			color = cSettings::GetServiceChatColor();
			font = cSettings::GetServiceChatFont();
		break;
	};

	// move cursor to the end
	QTextCursor c = ui.lwLines->textCursor();
	c.movePosition(QTextCursor::End);
	ui.lwLines->setTextCursor(c);

	ui.lwLines->setTextColor(color);
	ui.lwLines->setCurrentFont(font);

	const QString display_text = (msg.GetDirection() == cMessage::ChatEvent) ? msg.GetText() : person_name + ": " + msg.GetText();
	ui.lwLines->insertPlainText(display_text + "\n");

	// auto-scroll to the bottom
	QScrollBar *sb = ui.lwLines->verticalScrollBar();
	sb->setValue(sb->maximum());
}

void cWidgetVisitorDoc::ShowPresetMessages()
{
	QList<QString> list = cSettings::GetPresetMessages();

	if (list.size() <= 0)
		return;

	QMenu menu("Visitor actions", this);
	connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(PresetActionTriggered(QAction*)));
	for (int i = 0; i < list.size(); ++i)
	{
		QString text = list[i];

		text.replace("[%NAME%]", visitor.GetNameByVisitor(), Qt::CaseInsensitive);
		text.replace("[%DATE%]", QDate::currentDate().toString(Qt::ISODate), Qt::CaseInsensitive);
		text.replace("[%TIME%]", QTime::currentTime().toString(Qt::ISODate), Qt::CaseInsensitive);

		QAction *action = new QAction(text, this);
		action->setData(QVariant(text));
		menu.addAction(action);
	}
	menu.exec(QCursor::pos());
}

void cWidgetVisitorDoc::PresetActionTriggered(QAction *action)
{
	const QString text = action->data().toString();
	ui.leMessage->insert(text);
}

void cWidgetVisitorDoc::ClearReport(QTableWidget *widget)
{
	if (widget == NULL)
		return;

	while (widget->rowCount() > 0)
		widget->removeRow(0);
}


//
// Chat History
//
void cWidgetVisitorDoc::chSearch()
{
	found_message_id = -1;
	ChatHistoryPage = 0;
	SearchString = ui.leSearchString->text();
	if (chDoSearch())
		chLatest();
}

void cWidgetVisitorDoc::chPageFirst()
{
	ChatHistoryPage = 0;
	chDoSearch();
}

void cWidgetVisitorDoc::chPagePrev()
{
	--ChatHistoryPage;
	chDoSearch();
}

void cWidgetVisitorDoc::chPageNext()
{
	++ChatHistoryPage;
	chDoSearch();
}

void cWidgetVisitorDoc::chPageLast()
{
	ChatHistoryPage = ChatHistoryPagesCount;
	chDoSearch();
}

bool cWidgetVisitorDoc::chDoSearch()
{
	bool matches_found = false;
	ClearReport(ui.chReport);

	list<CChatHistoryRecord> records = ChatHistory->GetRecords(visitor.GetId(), cGlobals::GetOperatorId(), SearchString, ChatHistoryPage);
	for (list<CChatHistoryRecord>::iterator it = records.begin(); it != records.end(); ++it)
	{
		const CChatHistoryRecord &record = *it;
		matches_found |= record.IsMatched();
		chDisplayRecord(record);
	}

	const uint_t RecordsCount = ChatHistory->GetRecordsCount(visitor.GetId(), cGlobals::GetOperatorId()/*, SearchString*/);
	ChatHistoryPagesCount = RecordsCount / CHAT_LOG_MESSAGES_PER_PAGE;
	const bool equal = (RecordsCount % CHAT_LOG_MESSAGES_PER_PAGE == 0) ? true : false;
	chSetupPager((equal) ? ChatHistoryPagesCount : ChatHistoryPagesCount+1);

	const bool nav_back = ChatHistoryPagesCount > 0 && ChatHistoryPage > 0;
	const bool nav_forw = ChatHistoryPagesCount > 0 && ChatHistoryPage < ((equal) ? ChatHistoryPagesCount-1 : ChatHistoryPagesCount);
	ui.pbPageFirst->setEnabled(nav_back);
	ui.pbPagePrev->setEnabled(nav_back);
	ui.pbPageLast->setEnabled(nav_forw);
	ui.pbPageNext->setEnabled(nav_forw);

	// Navigation in pattern matches
	const bool patt_nav = (RecordsCount > 0 && SearchString.length() > 0 && matches_found);
	ui.chLatest->setEnabled(patt_nav);
	ui.chNext->setEnabled(patt_nav);
	ui.chPrevious->setEnabled(patt_nav);

	return matches_found;
}

void cWidgetVisitorDoc::chDisplayRecord(const CChatHistoryRecord &record)
{
	QTableWidgetItem *item = NULL;

	const int insertIndex = ui.chReport->rowCount();
	ui.chReport->insertRow(insertIndex);

	QColor row_color;
	if (record.GetId() == found_message_id)
		row_color = Qt::red;
	else if (record.IsMatched())
		row_color = QColor::fromRgb(230, 230, 230);
	else
		row_color = Qt::white;

	// Date
	item = new QTableWidgetItem(record.GetDateString(), QTableWidgetItem::Type);
	item->setBackgroundColor(row_color);
	ui.chReport->setItem(insertIndex, CHAT_HISTORY_COLUMN_INDEX_DATE, item);

	// Name
	const QString name = (record.GetDirection() == CChatHistoryRecord::VisitorToOperator) ? visitor.GetCombinedName() : cGlobals::GetAssignedOperatorName();
	item = new QTableWidgetItem(name, QTableWidgetItem::Type);
	item->setBackgroundColor(row_color);
	ui.chReport->setItem(insertIndex, CHAT_HISTORY_COLUMN_INDEX_NAME, item);

	// Text
	item = new QTableWidgetItem(record.GetString(), QTableWidgetItem::Type);
	item->setBackgroundColor(row_color);
	ui.chReport->setItem(insertIndex, CHAT_HISTORY_COLUMN_INDEX_TEXT, item);
}

void cWidgetVisitorDoc::chSetupPager(const uint_t PagesCount)
{
	if (PagesCount == 0)
	{
		ui.lPages->setText("0/0");
		return;
	}

	ui.lPages->setText(QString::number(ChatHistoryPage + 1) + "/" + QString::number(PagesCount));
}

void cWidgetVisitorDoc::chClearAll()
{
//	if (QMessageBox::question(this, "Clear Chat History", "Are you sure?", "&Yes", "No", 0, 1, 0) == 0)
	if (DialogAreYouSure(this, tr("Clear Chat History", "MSG_CLEAR_CHAT_HISTORY")) == 0)
	{
		const uint_t visitor_id = visitor.GetId();
		ChatHistory->ClearRecords(visitor_id);
		chSearch();
	}
}

void cWidgetVisitorDoc::chNext()
{
//	ChatHistoryPage = 0;
	SearchString = ui.leSearchString->text();

	uint_t page = 0;
	const long id = ChatHistory->FindNextId(visitor.GetId(), cGlobals::GetOperatorId(), SearchString, found_message_id, page);
	if (id >= 0)
	{
		ChatHistoryPage = page;
		found_message_id = id;
	}

	chDoSearch();
}

void cWidgetVisitorDoc::chPrevious()
{
//	ChatHistoryPage = 0;
	SearchString = ui.leSearchString->text();

	uint_t page = 0;
	const long id = ChatHistory->FindPreviousId(visitor.GetId(), cGlobals::GetOperatorId(), SearchString, found_message_id, page);
	if (id >= 0)
	{
		ChatHistoryPage = page;
		found_message_id = id;
	}

	chDoSearch();
}

void cWidgetVisitorDoc::chLatest()
{
//	ChatHistoryPage = 0;
	SearchString = ui.leSearchString->text();

	uint_t page = 0;
	const long id = ChatHistory->FindLatestId(visitor.GetId(), cGlobals::GetOperatorId(), SearchString, page);
	if (id >= 0)
	{
		ChatHistoryPage = page;
		found_message_id = id;
	}

	chDoSearch();
}


//
// Event Log
//
void cWidgetVisitorDoc::elSearch()
{
	EventLogPage = 0;
	elDateStart = ui.deEventLogDateStart->date();
	elDateEnd = ui.deEventLogDateEnd->date();
	elPattern = ui.leEventPattern->text();

//	elEvents
	elEvents.clear();
	for (int i = 0; i < ui.lwEventLogEvents->count(); ++i)
	{
		QListWidgetItem *item = ui.lwEventLogEvents->item(i);
		if (item->isSelected())
		{
			const QVariant &var = item->data(Qt::UserRole);
			elEvents.push_back(var.toInt());
		}
	}

	elDoSearch();
}

void cWidgetVisitorDoc::elPageFirst()
{
	EventLogPage = 0;
	elDoSearch();
}

void cWidgetVisitorDoc::elPagePrev()
{
	--EventLogPage;
	elDoSearch();
}

void cWidgetVisitorDoc::elPageNext()
{
	++EventLogPage;
	elDoSearch();
}

void cWidgetVisitorDoc::elPageLast()
{
	EventLogPage = EventLogPagesCount;
	elDoSearch();
}



void cWidgetVisitorDoc::elDoSearch()
{
//	elClearReport();
	ClearReport(ui.twEventLogResult);

	EventLogRecords records = EventLog->GetRecords(visitor.GetId(), cGlobals::GetOperatorId(), elDateStart, elDateEnd, elPattern, elEvents, EventLogPage);
	for (EventLogRecords::iterator it = records.begin(); it != records.end(); ++it)
		elDisplayRecord(*it);

	const uint_t RecordsCount = EventLog->GetRecordsCount(visitor.GetId(), cGlobals::GetOperatorId(), elDateStart, elDateEnd, elPattern, elEvents);
	EventLogPagesCount = RecordsCount / CHAT_LOG_MESSAGES_PER_PAGE;
	const bool equal = (RecordsCount % CHAT_LOG_MESSAGES_PER_PAGE == 0) ? true : false;
	elSetupPager((equal) ? EventLogPagesCount : EventLogPagesCount+1);

	const bool nav_back = EventLogPagesCount > 0 && EventLogPage > 0;
	const bool nav_forw = EventLogPagesCount > 0 && EventLogPage < ((equal) ? EventLogPagesCount-1 : EventLogPagesCount);
	ui.pbEventLogFirst->setEnabled(nav_back);
	ui.pbEventLogPrev->setEnabled(nav_back);
	ui.pbEventLogLast->setEnabled(nav_forw);
	ui.pbEventLogNext->setEnabled(nav_forw);
}

void cWidgetVisitorDoc::elDisplayRecord(const CEventLogRecord &record)
{
	QTableWidgetItem *item = NULL;

	const int insertIndex = ui.twEventLogResult->rowCount();
	ui.twEventLogResult->insertRow(insertIndex);

	// Date
	item = new QTableWidgetItem(record.GetDateString(), QTableWidgetItem::Type);
	ui.twEventLogResult->setItem(insertIndex, EVENT_LOG_COLUMN_INDEX_DATE, item);

	// Name
	const QString display_name = (record.GetOperatorName() == "") ? "-" : record.GetOperatorName();
	item = new QTableWidgetItem(display_name, QTableWidgetItem::Type);
	ui.twEventLogResult->setItem(insertIndex, EVENT_LOG_COLUMN_INDEX_NAME, item);

	// Text
	const QString text = (record.GetEvent() == CEventLogRecord::PageChanged) ? record.GetPageURL() + "\n[ " + record.GetPageTitle() + " ]" : record.GetEventDesc();
	item = new QTableWidgetItem(text, QTableWidgetItem::Type);
	ui.twEventLogResult->setItem(insertIndex, EVENT_LOG_COLUMN_INDEX_TEXT, item);
}

void cWidgetVisitorDoc::elSetupPager(const uint_t PagesCount)
{
	if (PagesCount == 0)
	{
		ui.lEventLogPages->setText("0/0");
		return;
	}

	ui.lEventLogPages->setText(QString::number(EventLogPage + 1) + "/" + QString::number(PagesCount));
}

void cWidgetVisitorDoc::elClearAll()
{
//	if (QMessageBox::question(this, "Clear Event Logs", "Are you sure?", "&Yes", "No", 0, 1, 0) == 0)
	if (DialogAreYouSure(this, tr("Clear Event Logs", "MSG_CLEAR_EVENT_LOGS")) == 0)
	{
		const uint_t visitor_id = visitor.GetId();
		EventLog->ClearRecords(visitor_id, cGlobals::GetOperatorId());
		elSearch();
	}
}


// Notes
void cWidgetVisitorDoc::display_note_item(const CNote &note)
{
	QTreeWidgetItem *item = new QTreeWidgetItem();
	item->setText(NoteColumnDate, note.GetDateString());
	item->setText(NoteColumnId, QString::number(note.GetId()));
	item->setText(NoteColumnOperatorId, QString::number(note.GetOperatorId()));
	item->setText(NoteColumnAuthor, note.GetOperatorName());
	item->setText(NoteColumnTest, note.GetMessage());

	ui.twNotes->insertTopLevelItem(ui.twNotes->topLevelItemCount(), item);
}

void cWidgetVisitorDoc::noteAdd()
{
	CNote note(0);
	note.SetMessage(ui.ptNote->toPlainText());
	note.SetOperatorName(cGlobals::GetAssignedOperatorName());
	newNotes.push_back(note);
	display_note_item(note);

	ui.ptNote->clear();
}

void cWidgetVisitorDoc::noteDelete()
{
	QList<QTreeWidgetItem*> items = ui.twNotes->selectedItems();
	if (items.count() <= 0)
		return;

	QTreeWidgetItem *item = items.first();
	const uint_t note_id = item->text(NoteColumnId).toUInt();

//	if (QMessageBox::question(this, "Delete Note", "Are you sure?", "&Yes", "No", 0, 1, 0) != 0)
	if (DialogAreYouSure(this, tr("Delete Note", "MSG_DELETE_NOTE")) != 0)
		return;

	CNote note(note_id);
	obsoleteNotes.push_back(note);

	for (int i = 0; i < ui.twNotes->topLevelItemCount(); ++i)
	{
		if (ui.twNotes->topLevelItem(i) == item)
		{
			ui.twNotes->takeTopLevelItem(i);
			break;
		}
	}
}

void cWidgetVisitorDoc::noteTextChanged()
{
	ui.pbNoteAdd->setEnabled(ui.ptNote->toPlainText().length() > 0);
}

void cWidgetVisitorDoc::noteItemSelectionChanged()
{
	QList<QTreeWidgetItem*> items = ui.twNotes->selectedItems();
	if (items.count() <= 0)
		return;

	QTreeWidgetItem *item = items.first();
	const uint_t operator_id = item->text(NoteColumnOperatorId).toUInt();
	const bool allow_delete = (cGlobals::GetOperatorId() == operator_id);

	ui.pbNoteDelete->setEnabled(allow_delete);
}




// Files
void cWidgetVisitorDoc::display_file_item(const CUploadedFile &file)
{
	QTreeWidgetItem *item = new QTreeWidgetItem();
	item->setText(DownloadsColumnFilename, file.GetFilename());
	item->setText(DownloadsColumnId, QString::number(file.GetId()));
	item->setText(DownloadsColumnOperatorId, QString::number(file.GetOperatorId()));
	item->setText(DownloadsColumnState, QString::number(file.GetUploadState()));
	item->setText(DownloadsColumnOperator, file.GetOperatorName());
	item->setText(DownloadsColumnSize, file.GetSizeString());
	item->setText(DownloadsColumnDate, file.GetDateString());

	ui.twDownloads->insertTopLevelItem(ui.twDownloads->topLevelItemCount(), item);
}

void cWidgetVisitorDoc::downloadsDelete()
{
	QList<QTreeWidgetItem*> items = ui.twDownloads->selectedItems();
	if (items.count() <= 0)
		return;

	if (!cGlobals::IsConnected())
	{
		QMessageBox::question(this, tr("Not connected", "MSG_NOT_CONNECTED"), tr("No connection to the store", "MSG_NOT_CONNECTED_STORE"), "&OK", 0, 0);
		return;
	}

	QTreeWidgetItem *item = items.first();
	const uint_t file_id = item->text(DownloadsColumnId).toUInt();

//	if (QMessageBox::question(this, "Delete Uploaded File", "Are you sure?", "&Yes", "No", 0, 1, 0) != 0)
	if (DialogAreYouSure(this, tr("Delete Uploaded File", "MSG_DELETE_UPLOADED_FILE")) != 0)
		return;

	CUploadedFile file(file_id);
	obsoleteUploadedFiles.push_back(file);

	for (int i = 0; i < ui.twDownloads->topLevelItemCount(); ++i)
	{
		if (ui.twDownloads->topLevelItem(i) == item)
		{
			ui.twDownloads->takeTopLevelItem(i);
			break;
		}
	}
}

void cWidgetVisitorDoc::download_get_states(bool &allow_delete, bool &allow_upload)
{
	allow_delete = false;
	allow_upload = false;

	QList<QTreeWidgetItem*> items = ui.twDownloads->selectedItems();
	if (items.count() <= 0)
		return;

	QTreeWidgetItem *item = items.first();
	const uint_t operator_id = item->text(DownloadsColumnOperatorId).toUInt();
	const CUploadedFile::UploadState state = static_cast<CUploadedFile::UploadState>(item->text(DownloadsColumnState).toUInt());

	allow_delete = (cGlobals::GetOperatorId() == operator_id);
	allow_upload = state == CUploadedFile::UploadStateUploaded;
}

void cWidgetVisitorDoc::downloadsItemSelectionChanged()
{
	bool allow_delete, allow_upload;
	download_get_states(allow_delete, allow_upload);

	ui.pbDownloadsDelete->setEnabled(allow_delete);
	ui.pbDownloadsPost->setEnabled(allow_upload);
}

void cWidgetVisitorDoc::downloadsUpload()
{
	if (!cGlobals::IsConnected())
	{
//		QMessageBox::question(this, "Not connected", "No connection to the store", "&OK", 0, 0);
		QMessageBox::question(this, tr("Not connected", "MSG_NOT_CONNECTED"), tr("No connection to the store", "MSG_NOT_CONNECTED_STORE"), "&OK", 0, 0);
		return;
	}

	QFileDialog dialog;
	dialog.setFileMode(QFileDialog::ExistingFiles);
	if (dialog.exec() != QDialog::Accepted)
		return;

	const cConnectionParams &params = cGlobals::GetConnectionParams();
	const QString login = cGlobals::GetLogin();
	const QString password = cGlobals::GetPassword();

	QStringList files = dialog.selectedFiles();
	for (int i = 0; i < files.size(); ++i)
	{
		CFileUploader *uploader = new CFileUploader(params);
		uploader->UploadFile(login, password, visitor.GetId(), files[i]);
	}
}

void cWidgetVisitorDoc::downloadsPostToChat()
{
	QList<QTreeWidgetItem*> items = ui.twDownloads->selectedItems();
	if (items.count() <= 0)
		return;

	if (!cGlobals::IsConnected())
	{
		QMessageBox::question(this, tr("Not connected", "MSG_NOT_CONNECTED"), tr("No connection to the store", "MSG_NOT_CONNECTED_STORE"), "&OK", 0, 0);
		return;
	}

	QTreeWidgetItem *item = items.first();
	const uint_t file_id = item->text(DownloadsColumnId).toUInt();
	const CUploadedFile::UploadState state = static_cast<CUploadedFile::UploadState>(item->text(DownloadsColumnState).toUInt());

	if (state != CUploadedFile::UploadStateUploaded)
	{
//		QMessageBox::critical(this, "Error", "File not uploaded to the server", QMessageBox::Ok);
		QMessageBox::critical(this, tr("Error", "MSG_ERROR"), tr("File not uploaded to the server", "MSG_FILE_NOT_UPLOADED"), QMessageBox::Ok);
		return;
	}

//	if (QMessageBox::question(this, "Post file to visitor", "Are you sure?", "&Yes", "No", 0, 1, 0) != 0)
	if (DialogAreYouSure(this, tr("Post file to the visitor", "MSG_FILE_POST_TO_VISITOR")) != 0)
		return;

	CUploadedFile file(file_id);

	postUploadedFiles.push_back(file);
}


void cWidgetVisitorDoc::downloadContextMenu(const QPoint &pt)
{
	disconnect(this, SLOT(downloadContextPost()));
	disconnect(this, SLOT(downloadContextDelete()));

	// check states
	bool allow_delete, allow_upload;
	download_get_states(allow_delete, allow_upload);
	if (!allow_upload)
		return;

	if (allow_delete == false && allow_upload == false)
		return;

	QMenu menu("Shortcut actions", this);

	if (allow_upload)
	{
		QAction *post_action = new QAction(QIcon(":/lh3/Resources/transfer.png"), QApplication::trUtf8("Post to Visitor", "LBL_DLG_VISITOR_DOWNLOADS_POST_FILE"), this);
		connect(post_action, SIGNAL(triggered(bool)), this, SLOT(downloadContextPost(bool)));
		menu.addAction(post_action);
	}

	if (allow_delete)
	{
		QAction *delete_action = new QAction(QIcon(":/lh3/Resources/delete.png"), QApplication::trUtf8("Delete", "LBL_DELETE"), this);
		connect(delete_action, SIGNAL(triggered(bool)), this, SLOT(downloadContextDelete(bool)));
		menu.addAction(delete_action);
	}
	
	menu.exec(QCursor::pos());
}


