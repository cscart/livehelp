#include "stdafx.h"
#include "DialogConfig.h"

cDialogConfig::cDialogConfig()
{
	ui.setupUi(this);

#if defined(Q_OS_MAC)
        // make some style changes for Mac OS
        QPoint wpos = ui.widgetDummy1->pos();
        ui.widgetDummy1->move(QPoint(wpos.x(), wpos.y() + 10));

        wpos = ui.widgetDummy2->pos();
        ui.widgetDummy2->move(QPoint(wpos.x(), wpos.y() + 10));
#endif

	PageIndex pages[] = {ConnectionParams, PresetMessages, History, ColorsFonts, Shortcuts, PageSounds, Tray, PageLanguages};
	for (int i = 0; i < sizeof(pages) / sizeof(pages[0]); ++i)
		ui.listWidget->item(i)->setData(Qt::UserRole, pages[i]);

	connect(&manager, SIGNAL(OnTestConnectSuccess()), SLOT(OnTestConnectSuccess()));
	connect(&manager, SIGNAL(OnTestConnectError(QNetworkReply::NetworkError, const QString&, bool)), SLOT(OnTestConnectError(QNetworkReply::NetworkError, const QString&, bool)));


	urlIcon = QIcon(":/lh3/Resources/url.png");

	// read all settings in temporary variables
	connections = cSettings::GetConnectionParamsList();
	proxySettings = cSettings::GetProxyParams();

	// chat colors
	m_ChatColors[ChatColorVisitor] = cSettings::GetVisitorChatColor();
	m_ChatColors[ChatColorOperator] = cSettings::GetOperatorChatColor();
	m_ChatColors[ChatColorService] = cSettings::GetServiceChatColor();

	// chat fonts
	m_ChatFonts[ChatFontVisitor] = cSettings::GetVisitorChatFont();
	m_ChatFonts[ChatFontOperator] = cSettings::GetOperatorChatFont();
	m_ChatFonts[ChatFontService] = cSettings::GetServiceChatFont();

	// history
	m_HistoryKeepDays = cSettings::GetHistoryKeepingDays();

	// raise
	m_RaiseEvents[RaiseNewVisitor] = cSettings::IsRaiseNewVisitor();
	m_RaiseEvents[RaiseNewOfflineMessage] = cSettings::IsRaiseNewOfflineMessage();
	m_RaiseEvents[RaiseNewMessage] = cSettings::IsRaiseNewMessage();
	m_RaiseEvents[RaiseWaitingChat] = cSettings::IsRaiseWaitingChat();
	hide_on_close = cSettings::IsHideOnProgrammClose();

	// sounds
	Sounds[SoundNewVisitor].enabled = cSettings::IsSoundNewVisitor();
	Sounds[SoundNewVisitor].filename = cSettings::GetSoundNewVisitorFile();

	Sounds[SoundWaitingChat].enabled = cSettings::IsSoundWaitingChat();
	Sounds[SoundWaitingChat].filename = cSettings::GetSoundWaitingChatFile();

	Sounds[SoundNewReplyMessage].enabled = cSettings::IsSoundNewReplyMessage();
	Sounds[SoundNewReplyMessage].filename = cSettings::GetSoundNewReplyMessageFile();

	Sounds[SoundNewOfflineMessage].enabled = cSettings::IsSoundNewOfflineMessage();
	Sounds[SoundNewOfflineMessage].filename = cSettings::GetSoundNewOfflineMessageFile();

	Sounds[SoundAwayVisitor].enabled = cSettings::IsSoundAwayVisitor();
	Sounds[SoundAwayVisitor].filename = cSettings::GetSoundAwayVisitorFile();

	// Languages
	lng_filename = cSettings::GetLanguage();

	// prepare UI
	prepare_pages();
}

cDialogConfig::~cDialogConfig()
{
}

void cDialogConfig::pageChanged(QListWidgetItem *new_item,QListWidgetItem *old_item)
{
	const int index = static_cast<int>(new_item->data(Qt::UserRole).toInt());
	ui.stackedWidget->setCurrentIndex(index);
}

void cDialogConfig::prepare_pages()
{
 // fill all UI pages with settings

	// connections
	for (int i = 0; i < connections.size(); ++i)
	{
		const cConnectionParams &params = connections[i];
		ui.cbStoreAlias->addItem(urlIcon, params.GetAlias());
	}
	connection_ui_enabled(connections.size() > 0);

	// Proxy params
	cProxyParams proxy = cSettings::GetProxyParams();
	ui.groupBox_Proxy->setChecked(proxy.IsUseProxy());
	ui.leProxyUrl->setText(proxy.GetProxyUrl());
	ui.leProxyUser->setText(proxy.GetProxyUsername());
	ui.leProxyPassword->setText(proxy.GetProxyPassword());
	ui.leProxyPort->setValue(proxy.GetProxyPort());


	// Prepare preset messages
	QList<QString> list = cSettings::GetPresetMessages();
	ui.twMessages->setRowCount(list.size());
	for (int i = 0; i < list.size(); ++i)
	{

		QTableWidgetItem *item = new QTableWidgetItem();
		item->setText(list[i]);
		ui.twMessages->setItem(i, 0, item);
	}


	// prepare Colors & Fonts
	ui.pbVColor->setStyleSheet("background-color: " + rgb_string(m_ChatColors[ChatColorVisitor]));
	ui.pbOColor->setStyleSheet("background-color: " + rgb_string(m_ChatColors[ChatColorOperator]));
	ui.pbSColor->setStyleSheet("background-color: " + rgb_string(m_ChatColors[ChatColorService]));

	ui.leVFont->setFont(m_ChatFonts[ChatFontVisitor]);
	ui.leOFont->setFont(m_ChatFonts[ChatFontOperator]);
	ui.leSFont->setFont(m_ChatFonts[ChatFontService]);


	// prepare raise
	ui.cbRaiseNewVisitor->setChecked(m_RaiseEvents[RaiseNewVisitor]);
	ui.cbRaiseNewOffMessage->setChecked(m_RaiseEvents[RaiseNewOfflineMessage]);
	ui.cbRaiseNewMessage->setChecked(m_RaiseEvents[RaiseNewMessage]);
	ui.cbRaiseWaitingChat->setChecked(m_RaiseEvents[RaiseWaitingChat]);
	ui.cbHideProgrammClose->setChecked(hide_on_close);

	// sounds
	ui.cbSoundNewVisitor->setChecked(Sounds[SoundNewVisitor].enabled);
	ui.leSoundNewVisitor->setText(Sounds[SoundNewVisitor].filename);

	ui.cbSoundWaitForChat->setChecked(Sounds[SoundWaitingChat].enabled);
	ui.leSoundWaitChat->setText(Sounds[SoundWaitingChat].filename);

	ui.cbSoundNewReplyMessage->setChecked(Sounds[SoundNewReplyMessage].enabled);
	ui.leSoundNewReplayMessage->setText(Sounds[SoundNewReplyMessage].filename);

	ui.cbSoundNewOfflineMessage->setChecked(Sounds[SoundNewOfflineMessage].enabled);
	ui.leSoundNewOfflineMessage->setText(Sounds[SoundNewOfflineMessage].filename);

	ui.cbSoundAwayVisitor->setChecked(Sounds[SoundAwayVisitor].enabled);
	ui.leSoundAwayVisitore->setText(Sounds[SoundAwayVisitor].filename);

	// history
	ui.sbHistoryDays->setValue(m_HistoryKeepDays);

	// Shortcuts
	for (int i = 0; i < ShortcutsCount; ++i)
	{
		const QKeySequence shortcut = cSettings::GetShortcut(static_cast<ShortcutType>(i));

		QTableWidgetItem *item = new QTableWidgetItem(((shortcut.isEmpty()) ? tr("Empty", "MSG_EMPTY") : shortcut.toString()), QTableWidgetItem::UserType);
		item->setData(Qt::UserRole, shortcut);
		ui.twShortcuts->setItem(i, ShortcutColumnShortcut, item);
	}

	// Languages
	LanguageInfoArray languages(CTranslator::GetLanguages());
	for (int i = 0; i < languages.count(); ++i)
	{
		const LanguageInfo &lang = languages[i];
		QListWidgetItem *item = new QListWidgetItem(lang.name);
		item->setData(Qt::UserRole, lang.filename);
		ui.lwLanguages->addItem(item);
		if (lang.filename == lng_filename)
			ui.lwLanguages->setItemSelected(item, true);
	}
}

QString cDialogConfig::rgb_string(const QColor &color) const
{
	return QString("rgb(") + QString::number(color.red()) + QString(", ") + QString::number(color.green()) + QString(", ") + QString::number(color.blue()) + QString(")");
}

void cDialogConfig::apply_settings()
{
	// get settings from the UI then apply them

	compose_connection_params();

	// connections
	cSettings::SetConnectionParamsList(connections);

	// proxy
	cSettings::SetProxyParams(proxySettings);
//	cSettings::ApplyGlobalProxySettings();

	// Apply preset messages
	QList<QString> list;
	for (int i = 0; i < ui.twMessages->rowCount(); ++i)
	{
		QTableWidgetItem *item = ui.twMessages->item(i, 0);
		if (item != NULL)
			list.push_back(item->text());
	}
	cSettings::SetPresetMessages(list);


	// apply chat colors
	cSettings::SetVisitorChatColor(m_ChatColors[ChatColorVisitor]);
	cSettings::SetOperatorChatColor(m_ChatColors[ChatColorOperator]);
	cSettings::SetServiceChatColor(m_ChatColors[ChatColorService]);

	// apply chat fonts
	cSettings::SetVisitorChatFont(m_ChatFonts[ChatFontVisitor]);
	cSettings::SetOperatorChatFont(m_ChatFonts[ChatFontOperator]);
	cSettings::SetServiceChatFont(m_ChatFonts[ChatFontService]);

	// history
	cSettings::SetHistoryKeepingDays(m_HistoryKeepDays);

	// raise
	cSettings::SetRaiseNewVisitor(m_RaiseEvents[RaiseNewVisitor]);
	cSettings::SetRaiseNewOfflineMessage(m_RaiseEvents[RaiseNewOfflineMessage]);
	cSettings::SetRaiseNewMessage(m_RaiseEvents[RaiseNewMessage]);
	cSettings::SetRaiseWaitingChat(m_RaiseEvents[RaiseWaitingChat]);
	cSettings::SetHideOnProgrammClose(hide_on_close);

	// sounds
	cSettings::SetSoundNewVisitor(Sounds[SoundNewVisitor].enabled);
	cSettings::SetSoundNewVisitorFile(Sounds[SoundNewVisitor].filename);

	cSettings::SetSoundWaitingChat(Sounds[SoundWaitingChat].enabled);
	cSettings::SetSoundWaitingChatFile(Sounds[SoundWaitingChat].filename);

	cSettings::SetSoundNewReplyMessage(Sounds[SoundNewReplyMessage].enabled);
	cSettings::SetSoundNewReplyMessageFile(Sounds[SoundNewReplyMessage].filename);

	cSettings::SetSoundNewOfflineMessage(Sounds[SoundNewOfflineMessage].enabled);
	cSettings::SetSoundNewOfflineMessageFile(Sounds[SoundNewOfflineMessage].filename);

	cSettings::SetSoundAwayVisitor(Sounds[SoundAwayVisitor].enabled);
	cSettings::SetSoundAwayVisitorFile(Sounds[SoundAwayVisitor].filename);


	// Shortcuts
	for (int i = 0; i < ShortcutsCount; ++i)
	{
		QTableWidgetItem *item = ui.twShortcuts->item(i, ShortcutColumnShortcut);
		const QKeySequence shortcut = item->data(Qt::UserRole).value<QKeySequence>();
		cSettings::SetShortcut(static_cast<ShortcutType>(i), shortcut);
	}

	// Languages
	cSettings::SetLanguage(lng_filename);


	// send 'settings applied' signal
	OnSettingsApplied();
}

void cDialogConfig::OnButtonApply()
{
	apply_settings();
}

void cDialogConfig::OnButtonOK()
{
	apply_settings();
	close();
}



/////////////////////////// Connection Settings ///////////////////////////

void cDialogConfig::OnTestConnectError(QNetworkReply::NetworkError code, const QString &desc, bool auth)
{
	if (auth)
//		QMessageBox::critical(this, "Error", "Can not find installed shop with LiveHelp module.", QMessageBox::Ok);
		QMessageBox::critical(this, tr("Error", "MSG_ERROR"),
			tr("Can not find installed shop with LiveHelp module.", "MSG_CANT_FIND_SHOP"), QMessageBox::Ok);
	else
//		QMessageBox::critical(this, "Error", "Connection Error: " + desc, QMessageBox::Ok);
		QMessageBox::critical(this, tr("Error", "MSG_ERROR"), tr("Connection Error: ", "MSG_CONNECTION_ERROR") + desc, QMessageBox::Ok);
}

void cDialogConfig::OnTestConnectSuccess()
{
//	QMessageBox::information(this, "Success", "Connection test successfully passed.", QMessageBox::Ok);
	QMessageBox::information(this, tr("Success", "MSG_SUCCESS"), tr("Connection test successfully passed.", "MSG_CONNECTION_TEST_OK"), QMessageBox::Ok);
}

void cDialogConfig::TestConnection()
{
	// do test connection with local Proxy and connection settings

	const int i = ui.cbStoreAlias->currentIndex();
	if (i < 0 || i >= connections.size() || i >= ui.cbStoreAlias->count())
		return;

	compose_connection_params();
	const cConnectionParams &params = connections.at(i);

	manager.TestConnection(this, params, proxySettings);
}



void cDialogConfig::AddConnection()
{
	cConnectionParams params;
	connections.push_back(params);

	ui.cbStoreAlias->addItem(urlIcon, params.GetAlias());
	ui.cbStoreAlias->setCurrentIndex(ui.cbStoreAlias->count() - 1);
}

void cDialogConfig::DeleteConnection()
{
//	if (QMessageBox::question(this, "Settings", "Are you sure?", "&Yes", "No", 0, 1, 0) != 0)
	if (DialogAreYouSure(this, tr("Delete connection", "MSG_DELETE_CONNECTION")) != 0)
		return;

	const int i = ui.cbStoreAlias->currentIndex();
	if (i < 0 || i >= connections.size() || i >= ui.cbStoreAlias->count())
		return;

	connections.removeAt(i);
	ui.cbStoreAlias->removeItem(i);
}


void cDialogConfig::selectConnection(const int index)
{
	connection_ui_enabled((index < 0) ? false : true);

	if (index < 0 || index >= connections.size() || index >= ui.cbStoreAlias->count())
		return;

	static int lastIndex = index;

	if (lastIndex == index)
	{
		fill_connection_params(index);
		return;
	}

	compose_connection_params(lastIndex);
	fill_connection_params(index);

	lastIndex = index;
}

void cDialogConfig::storeUrlChanged(const QString &text)
{
	const int i = ui.cbStoreAlias->currentIndex();
	if (i < 0 || i >= connections.size() || i >= ui.cbStoreAlias->count())
		return;

//	ui.cbStoreAlias->setItemText(i, text);
}

void cDialogConfig::storeAliasChanged(const QString &text)
{
	const int i = ui.cbStoreAlias->currentIndex();
	if (i < 0 || i >= connections.size() || i >= ui.cbStoreAlias->count())
		return;

	ui.cbStoreAlias->setItemText(i, text);
}

void cDialogConfig::compose_connection_params(const int i)
{
	if (i < 0 || i >= connections.size() || i >= ui.cbStoreAlias->count())
		return;

	// connections
	cConnectionParams params;
	params.SetUrl(QUrl(ui.leStoreUrl->text()));
	params.SetAlias(ui.leStoreAlias->text());
	params.SetUseAuth(ui.cbUseAuth->isChecked());
	params.SetAuthUsername(ui.leAuthUser->text());
	params.SetAuthPassword(ui.leAuthPassword->text());
	connections.replace(i, params);

	// proxy
	proxySettings.SetUseProxy(ui.groupBox_Proxy->isChecked());
	proxySettings.SetProxyUrl(ui.leProxyUrl->text());
	proxySettings.SetProxyUsername(ui.leProxyUser->text());
	proxySettings.SetProxyPassword(ui.leProxyPassword->text());
	proxySettings.SetProxyPort(ui.leProxyPort->text().toInt());

}

void cDialogConfig::fill_connection_params(const int i)
{
	if (i < 0 || i >= connections.size() || i >= ui.cbStoreAlias->count())
		return;

	// connections
	const cConnectionParams &params = connections.at(i);
	ui.leStoreUrl->setText(params.GetUrl().toString());
	ui.leStoreAlias->setText(params.GetAlias());
	ui.cbUseAuth->setChecked(params.IsUseAuth());
	ui.leAuthUser->setText(params.GetAuthUsername());
	ui.leAuthPassword->setText(params.GetAuthPassword());
}

void cDialogConfig::connection_ui_enabled(const bool val)
{
        ui.connGroupBox->setEnabled(val);
	ui.pbTestDelete->setEnabled(val);
	ui.pbTestConnection->setEnabled(val);
}


/////////////////////////// Preset Messages ///////////////////////////

void cDialogConfig::AddPresetMessage()
{
	const int row = ui.twMessages->rowCount();
	ui.twMessages->insertRow(row);

	QTableWidgetItem *item = new QTableWidgetItem(QString());
	ui.twMessages->setItem(row, 0, item);
	ui.twMessages->editItem(item);
}

void cDialogConfig::DeletePresetMessage()
{
	QList<QTableWidgetItem*> items = ui.twMessages->selectedItems();

	for (QList<QTableWidgetItem*>::iterator it = items.begin(); it != items.end(); ++it)
	{
		const int row_index = (*it)->row();
		ui.twMessages->removeRow(row_index);
	}
}



//
// Colors & Fonts
//

void cDialogConfig::SelectVisitorTextColor()
{
	QColorDialog dialog;
	dialog.setCurrentColor(m_ChatColors[ChatColorVisitor]);
	if (dialog.exec() == QDialog::Accepted)
	{
		m_ChatColors[ChatColorVisitor] = dialog.selectedColor();
		ui.pbVColor->setStyleSheet("background-color: " + rgb_string(dialog.selectedColor()));
	}
}
void cDialogConfig::SelectOperatorTextColor()
{
	QColorDialog dialog;
	dialog.setCurrentColor(m_ChatColors[ChatColorOperator]);
	if (dialog.exec() == QDialog::Accepted)
	{
		m_ChatColors[ChatColorOperator] = dialog.selectedColor();
		ui.pbOColor->setStyleSheet("background-color: " + rgb_string(dialog.selectedColor()));
	}
}
void cDialogConfig::SelectServiceTextColor()
{
	QColorDialog dialog;
	dialog.setCurrentColor(m_ChatColors[ChatColorService]);
	if (dialog.exec() == QDialog::Accepted)
	{
		m_ChatColors[ChatColorService] = dialog.selectedColor();
		ui.pbSColor->setStyleSheet("background-color: " + rgb_string(dialog.selectedColor()));
	}
}

void cDialogConfig::SelectVisitorTextFont()
{
	bool ok;
	QFontDialog dialog;
	const QFont font = dialog.getFont(&ok, m_ChatFonts[ChatFontVisitor]);

	if (ok)
	{
		m_ChatFonts[ChatFontVisitor] = font;
		ui.leVFont->setFont(font);
	}
}
void cDialogConfig::SelectOperatorTextFont()
{
	bool ok;
	QFontDialog dialog;
	const QFont font = dialog.getFont(&ok, m_ChatFonts[ChatFontOperator]);

	if (ok)
	{
		m_ChatFonts[ChatFontOperator] = font;
		ui.leOFont->setFont(font);
	}
}
void cDialogConfig::SelectServiceTextFont()
{
	bool ok;
	QFontDialog dialog;
	const QFont font = dialog.getFont(&ok, m_ChatFonts[ChatFontService]);

	if (ok)
	{
		m_ChatFonts[ChatFontService] = font;
		ui.leSFont->setFont(font);
	}
}



//
// History
//
void cDialogConfig::HistoryKeepingChanged(int value)
{
	m_HistoryKeepDays = value;
}
void cDialogConfig::HistoryClear()
{
//	if (QMessageBox::question(this, "Clear history", "Are you sure to clear all Event logs and Chat history?", "&Yes", "No", 0, 1, 0) == 0)
	if (DialogAreYouSure(this, tr("Clear history", "MSG_CLEAR_HISTORY"),
		tr("Are you sure to clear all Event logs and Chat history?", "MSG_CLEAR_HISTORY_TEXT1")) != 0)
	{
		ChatHistory->ClearAll(0);
		EventLog->ClearAll(0);
	}
}


//
// Raise
//



//
// Sounds
//
void cDialogConfig::SelectSoundNewVisitor()
{
	const QString filename = QFileDialog::getOpenFileName();
	if (filename == "")
		return;

	Sounds[SoundNewVisitor].filename = filename;
	ui.leSoundNewVisitor->setText(filename);
}
void cDialogConfig::SelectSoundWaitingChat()
{
	const QString filename = QFileDialog::getOpenFileName();
	if (filename == "")
		return;

	Sounds[SoundWaitingChat].filename = filename;
	ui.leSoundWaitChat->setText(filename);
}
void cDialogConfig::SelectSoundNewReplyMessage()
{
	const QString filename = QFileDialog::getOpenFileName();
	if (filename == "")
		return;

	Sounds[SoundNewReplyMessage].filename = filename;
	ui.leSoundNewReplayMessage->setText(filename);
}
void cDialogConfig::SelectSoundNewOfflineMessage()
{
	const QString filename = QFileDialog::getOpenFileName();
	if (filename == "")
		return;

	Sounds[SoundNewOfflineMessage].filename = filename;
	ui.leSoundNewOfflineMessage->setText(filename);
}
void cDialogConfig::SelectSoundAwayVisitor()
{
	const QString filename = QFileDialog::getOpenFileName();
	if (filename == "")
		return;

	Sounds[SoundAwayVisitor].filename = filename;
	ui.leSoundAwayVisitore->setText(filename);
}

void cDialogConfig::PlaySoundNewVisitor()
{
	if( QSound::isAvailable())
		QSound::play(Sounds[SoundNewVisitor].filename);
}
void cDialogConfig::PlaySoundWaitingChat()
{
	if( QSound::isAvailable())
		QSound::play(Sounds[SoundWaitingChat].filename);
}
void cDialogConfig::PlaySoundNewReplyMessage()
{
	if( QSound::isAvailable())
		QSound::play(Sounds[SoundNewReplyMessage].filename);
}
void cDialogConfig::PlaySoundNewOfflineMessage()
{
	if( QSound::isAvailable())
		QSound::play(Sounds[SoundNewOfflineMessage].filename);
}
void cDialogConfig::PlaySoundAwayVisitor()
{
	if( QSound::isAvailable())
		QSound::play(Sounds[SoundAwayVisitor].filename);
}


// Shortcuts
void cDialogConfig::ShortcutItemClicked(QTableWidgetItem *item)
{
	if (item == NULL)
		return;

	// compose list of all exists shortcuts
	QSet<QString> exists_shortcuts;
	for (int i = 0; i < ShortcutsCount; ++i)
	{
		QTableWidgetItem *item = ui.twShortcuts->item(i, ShortcutColumnShortcut);
		QKeySequence shortcut = item->data(Qt::UserRole).value<QKeySequence>();
		if (!shortcut.isEmpty())
			exists_shortcuts.insert(shortcut.toString());
	}

	for (int i = 0; i < ShortcutsCount; ++i)
	{
		QTableWidgetItem *temp = ui.twShortcuts->item(i, ShortcutColumnShortcut);
		if (item == temp)
		{
			QKeySequence shortcut = item->data(Qt::UserRole).value<QKeySequence>();

			CDialogShortcut dialog(shortcut);
			dialog.SetExistsShortcuts(exists_shortcuts);
			dialog.exec();

			switch (dialog.GetCloseResult())
			{
				case CDialogShortcut::CloseAssign:
					shortcut = dialog.GetShortcut();
				break;

				case CDialogShortcut::CloseRemove:
					shortcut = QKeySequence();
				break;

				default:
				case CDialogShortcut::CloseCancel:
					return;
			}

			item->setData(Qt::UserRole, shortcut);
			item->setText(((shortcut.isEmpty()) ? tr("Empty", "MSG_EMPTY") : shortcut.toString()));

			break;
		}
	}
}

// Languages
void cDialogConfig::LanguageChanged(QListWidgetItem *new_item, QListWidgetItem *old_item)
{
	if (new_item == NULL)
		return;

	lng_filename = new_item->data(Qt::UserRole).toString();
	CTranslator::LoadLanguage(lng_filename);

	ui.retranslateUi(this);
	OnLanguageChanged(lng_filename);
}

void cDialogConfig::closeEvent(QCloseEvent *event)
{
	OnLanguageChanged(cSettings::GetLanguage());
}
