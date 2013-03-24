#pragma once

#include <QDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QSound>
#include "ui_DialogConfig.h"
#include "NetworkManager.h"
#include "Settings.h"
#include "DialogShortcut.h"
#include "ChatHistory.h"
#include "EventLog.h"
#include "Translator.h"

class cDialogConfig : public QDialog
{
	Q_OBJECT

private:
	enum PageIndex
	{
		ConnectionParams,
		PresetMessages,
		History,
		ColorsFonts,
		Shortcuts,
		PageSounds,
		Tray,
		PageLanguages
	};

	enum ChatColor
	{
		ChatColorVisitor,
		ChatColorOperator,
		ChatColorService,
		ChatColorCount
	};

	enum ChatFont
	{
		ChatFontVisitor,
		ChatFontOperator,
		ChatFontService,
		ChatFontCount
	};

	enum RaiseType
	{
		RaiseNewVisitor,
		RaiseNewOfflineMessage,
		RaiseNewMessage,
		RaiseWaitingChat,
		RaiseCount
	};

	enum SoundType
	{
		SoundNewVisitor,
		SoundWaitingChat,
		SoundNewReplyMessage,
		SoundNewOfflineMessage,
		SoundAwayVisitor,
		SoundCount
	};

	struct SoundNode
	{
		bool enabled;
		QString filename;
	};

	enum ShortcutColumns
	{
		ShortcutColumnName,
		ShortcutColumnShortcut
	};

private:
	Ui::DialogConfig ui;

	ConnectionParamsList connections;
	cProxyParams proxySettings;

	cNetworkManager manager;

	QIcon urlIcon;

	// fonts & colors
	QColor m_ChatColors[ChatColorCount];
	QFont m_ChatFonts[ChatFontCount];

	// history
	int m_HistoryKeepDays;

	//Raise
	bool m_RaiseEvents[RaiseCount];
	bool hide_on_close;

	// Sounds
	SoundNode Sounds[SoundCount];

	// Languages
	QString lng_filename;

	void apply_settings();

	void compose_connection_params() { compose_connection_params(ui.cbStoreAlias->currentIndex()); }
	void compose_connection_params(const int i);
	void fill_connection_params(const int i);

	void connection_ui_enabled(const bool val);

	void prepare_pages();


	QString rgb_string(const QColor &color) const;

protected:
		virtual void closeEvent(QCloseEvent *event);

public:
	cDialogConfig();
	~cDialogConfig();



private slots:;

	void OnButtonApply();
	void OnButtonOK();
	void OnButtonCancel() { close(); }


	void OnTestConnectError(QNetworkReply::NetworkError code, const QString &desc, bool auth);
	void OnTestConnectSuccess();


	void TestConnection();
	void AddConnection();
	void DeleteConnection();


	void selectConnection(const int index);
	void storeUrlChanged(const QString &text);
	void storeAliasChanged(const QString &text);


	void pageChanged(QListWidgetItem *new_item,QListWidgetItem *old_item);


	// Preset messages events
	void AddPresetMessage();
	void DeletePresetMessage();


	// Colors & Fonts
	void SelectVisitorTextColor();
	void SelectOperatorTextColor();
	void SelectServiceTextColor();

	void SelectVisitorTextFont();
	void SelectOperatorTextFont();
	void SelectServiceTextFont();


	// History
	void HistoryKeepingChanged(int);
	void HistoryClear();


	// Raise
	void RaiseNewVisitorToggled(bool value) { m_RaiseEvents[RaiseNewVisitor] = value; }
	void RaiseNewOfflineMessageToggled(bool value) { m_RaiseEvents[RaiseNewOfflineMessage] = value; }
	void RaiseNewMessageToggled(bool value) { m_RaiseEvents[RaiseNewMessage] = value; }
	void RaiseWaitingChatToggled(bool value) { m_RaiseEvents[RaiseWaitingChat] = value; }
	void HideProgrammCloseToggled(bool value) { hide_on_close = value; }


	// Sounds
	void SelectSoundNewVisitor();
	void SelectSoundWaitingChat();
	void SelectSoundNewReplyMessage();
	void SelectSoundNewOfflineMessage();
	void SelectSoundAwayVisitor();

	void PlaySoundNewVisitor();
	void PlaySoundWaitingChat();
	void PlaySoundNewReplyMessage();
	void PlaySoundNewOfflineMessage();
	void PlaySoundAwayVisitor();

	void EnableSoundNewVisitor(bool value) { Sounds[SoundNewVisitor].enabled = value; }
	void EnableSoundWaitingChat(bool value) { Sounds[SoundWaitingChat].enabled = value; }
	void EnableSoundNewReplyMessage(bool value) { Sounds[SoundNewReplyMessage].enabled = value; }
	void EnableSoundNewOfflineMessage(bool value) { Sounds[SoundNewOfflineMessage].enabled = value; }
	void EnableSoundAwayVisitor(bool value) { Sounds[SoundAwayVisitor].enabled = value; }


	// Shortcuts
	void ShortcutItemClicked(QTableWidgetItem *item);

	// Languages
	void LanguageChanged(QListWidgetItem *new_item, QListWidgetItem *old_item);

signals:;

	void OnSettingsApplied();

	void OnLanguageChanged(const QString &filename);

};
