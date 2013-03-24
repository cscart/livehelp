#ifndef LH3_H
#define LH3_H

#include <QtGui/QMainWindow>
#include "ui_lh3.h"

#include "DialogConnect.h"
#include "DialogAbout.h"
#include "DialogStickers.h"
#include "DialogOfflineMessages.h"
#include "DialogConfig.h"
#include "DialogVisitorRename.h"

#include "LayoutVisitorsTray.h"

#include "NetworkManager.h"
#include "VisitorsList.h"
#include "WidgetVisitorsTree.h"
#include "WidgetVisitorsMDI.h"
#include "WidgetDockStickers.h"
#include "WidgetDockOfflineMessages.h"

#include "Globals.h"

#include <QHttp>
#include <QMenu>
#include <QSystemTrayIcon>

#include "defines.h"

#include "ChatHistory.h"
#include "EventLog.h"

#include "Translator.h"

class lh3 : public QMainWindow, public ILanguageUI
{
	Q_OBJECT

protected:
	void update_window_caption();

	virtual void closeEvent(QCloseEvent *event);

public:
	lh3(QWidget *parent = 0, Qt::WFlags flags = 0);
	~lh3();

private:
	Ui::lh3Class ui;

	QMutex mutex;

	cNetworkManager network;
	cVisitorsList visitors;

	LayoutVisitorsTray *visitorsTray;
	cWidgetVisitorsTree *visitorsTree;
	cWidgetVisitorsMDI *visitorsMDI;
	CWidgetStickers *widgetStickers;
	CWidgetOfflineMessages *widgetOfflineMessages;

	CChatHistory _chat_history;
	CEventLog _event_log;

	QString store_name;

//	uint_t selected_visitor;


//	using QMainWindow::connect;

//	void updateVisitorActionToolbar();

	QSystemTrayIcon m_SystemTray;
	QMenu m_SystemTrayContextMenu;
	bool ignore_minimaze_on_close;

	void play_sound(const QString &filename);
	void apply_shortcuts_settings();
	void gc_history();

	// override
	virtual void ApplyLanguageLabels();

private slots:;
	void showAbout();

	void Connect();
	void Disconnect();

	void execConfig();


	void connected();
	void disconnected();
	void OnConnectionActive();
	void tryConnectError(QNetworkReply::NetworkError code, const QString &descr, bool auth_failed);
	void connectionError(QNetworkReply::NetworkError code, const QString &descr, bool auth_failed);


	// concurent action slots
	void respondReceivedXML(const CNodeXML &root);
	void openVisitorMDI(const uint_t visitor_id);

	void renameVisitor(const uint_t visitor_id);

	void EventNewVisitor();
	void EventWaitingVisitor();
	void EventAwayVisitor();
	void EventNewOfflineMessage();
	void EventNewChatMessage();

	void SystemTrayActivated(QSystemTrayIcon::ActivationReason reason);

	void ForceExit();
	void OnSettingsApplied();
	void OnLanguageChanged(const QString &filename);

signals:;
	void setConnected(bool);

};

#endif // LH3_H
