#include "stdafx.h"
#include "lh3.h"

lh3::lh3(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags), visitorsTray(NULL), m_SystemTray(QIcon(":/lh3/Resources/lh_icon.png")), ignore_minimaze_on_close(false)
{
	CTranslator::LoadLanguage(cSettings::GetLanguage());

	ui.setupUi(this);

//	ApplyLanguageLabels();

	// Prepare SQL connections
	ChatHistory->Open();
	EventLog->Open();

	update_window_caption();


	// create 'visitors tray'
	visitorsTray = new LayoutVisitorsTray(5, 5, 5);
	ui.widget_VisitorsTray_List->setLayout(visitorsTray);

	// create 'visitors tree'
	visitorsTree = new cWidgetVisitorsTree;
	ui.LayoutVisitorsTree->addWidget(visitorsTree);

	// create 'visitors MDI' area
	visitorsMDI = new cWidgetVisitorsMDI;
	ui.LayoutVisitorsMDI->addWidget(visitorsMDI);

	// create 'stickers' area
	widgetStickers = new CWidgetStickers();
	ui.LayoutStickers->addWidget(widgetStickers);

	// create 'stickers' area
	widgetOfflineMessages = new CWidgetOfflineMessages();
	ui.LayoutOfflineMessages->addWidget(widgetOfflineMessages);



	showMaximized();


	//	network
	connect(&network, SIGNAL(OnConnected()), this, SLOT(connected()));
	connect(&network, SIGNAL(OnDisconnected()), this, SLOT(disconnected()));
	connect(&network, SIGNAL(OnConnectionActive()), this, SLOT(OnConnectionActive()));
	connect(&network, SIGNAL(OnTryConnectError(QNetworkReply::NetworkError, const QString&, bool)), this, SLOT(tryConnectError(QNetworkReply::NetworkError, const QString&, bool)));
	connect(&network, SIGNAL(OnConnectionError(QNetworkReply::NetworkError, const QString&, bool)), this, SLOT(connectionError(QNetworkReply::NetworkError, const QString&, bool)));
//	connect(&network, SIGNAL(OnRespondReceivedXML(const QString&)), this, SLOT(respondReceivedXML(const QString&)));
	connect(&network, SIGNAL(OnRespondReceivedXML(const CNodeXML&)), this, SLOT(respondReceivedXML(const CNodeXML&)));

	// Compose visitors list DOM
//	connect(&network, SIGNAL(OnComposeVisitorsShort(QDomElement&)), &visitors, SLOT(GetVisitorsDOM(QDomElement&)));

	// requests to the shop
	connect(&network, SIGNAL(OnComposeRequest(QDomElement&)), &visitors, SLOT(GetVisitorsDOM(QDomElement&)));
	connect(&network, SIGNAL(OnComposeRequest(QDomElement&)), visitorsMDI, SLOT(GetVisitorParamsDOM(QDomElement&)));
	connect(&network, SIGNAL(OnComposeRequest(QDomElement&)), widgetStickers, SLOT(GetStickersDOM(QDomElement&)));
	connect(&network, SIGNAL(OnComposeRequest(QDomElement&)), widgetOfflineMessages, SLOT(GetMessagesDOM(QDomElement&)));

	// check for opened chat windows
	connect(&network, SIGNAL(OnCheckOpenedChat(bool&)), visitorsMDI, SLOT(HasOpenedChat(bool&)));

	// VisitorsTree
	connect(&visitors, SIGNAL(OnVisitorAdded(const cVisitor&, const bool)), visitorsTree, SLOT(AddVisitor(const cVisitor&, const bool)));
	connect(&visitors, SIGNAL(OnVisitorUpdated(const cVisitor&, const cVisitor&)), visitorsTree, SLOT(UpdateVisitor(const cVisitor&, const cVisitor&)));
	connect(&visitors, SIGNAL(OnVisitorRemoved(const cVisitor&)), visitorsTree, SLOT(RemoveVisitor(const cVisitor&)));

	// Visitors MDI
	connect(visitorsTree, SIGNAL(OnVisitorOpen(const uint_t)), this, SLOT(openVisitorMDI(const uint_t)));
	connect(visitorsTree, SIGNAL(OnVisitorRename(const uint_t)), this, SLOT(renameVisitor(const uint_t)));
	connect(&visitors, SIGNAL(OnVisitorAdded(const cVisitor&, const bool)), visitorsMDI, SLOT(UpdateVisitor(const cVisitor&, const bool)));
	connect(&visitors, SIGNAL(OnVisitorUpdated(const cVisitor&, const cVisitor&)), visitorsMDI, SLOT(UpdateVisitor(const cVisitor&, const cVisitor&)));
	connect(&visitors, SIGNAL(OnVisitorRemoved(const cVisitor&)), visitorsMDI, SLOT(RemoveVisitor(const cVisitor&)));
	connect(visitorsTray, SIGNAL(OnWindowSelect(const cVisitor&)), visitorsMDI, SLOT(SelectWindow(const cVisitor&)));

	// visitors tray
	connect(visitorsMDI, SIGNAL(OnOpenWindow(const cVisitor&)), visitorsTray, SLOT(OpenWindow(const cVisitor&)));
	connect(visitorsMDI, SIGNAL(OnUpdateWindow(const cVisitor&, const cVisitor&)), visitorsTray, SLOT(UpdateWindow(const cVisitor&, const cVisitor&)));
	connect(visitorsMDI, SIGNAL(OnCloseWindow(const uint_t)), visitorsTray, SLOT(CloseWindow(const uint_t)));
	connect(visitorsMDI, SIGNAL(OnWindowActivated(const uint_t)), visitorsTray, SLOT(WindowActivated(const uint_t)));
	connect(&visitors, SIGNAL(OnVisitorRemoved(const cVisitor&)), visitorsTray, SLOT(RemoveVisitor(const cVisitor&)));


	// manage MDI windows
	connect(ui.actionCascade, SIGNAL(activated()), visitorsMDI, SLOT(cascadeSubWindows()));
	connect(ui.actionTile, SIGNAL(activated()), visitorsMDI, SLOT(tileSubWindows()));
	connect(ui.actionClose_All, SIGNAL(activated()), visitorsMDI, SLOT(closeAll()));
	connect(ui.actionClose_Active, SIGNAL(activated()), visitorsMDI, SLOT(closeActive()));


	// events
	connect(&visitors, SIGNAL(EventNewVisitor()), this, SLOT(EventNewVisitor()));
	connect(&visitors, SIGNAL(EventWaitingVisitor()), this, SLOT(EventWaitingVisitor()));
	connect(&visitors, SIGNAL(EventAwayVisitor()), this, SLOT(EventAwayVisitor()));
	connect(widgetOfflineMessages, SIGNAL(EventNewOfflineMessage()), this, SLOT(EventNewOfflineMessage()));
	connect(visitorsMDI, SIGNAL(EventNewChatMessage()), this, SLOT(EventNewChatMessage()));

	// System tray
	connect(&m_SystemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(SystemTrayActivated(QSystemTrayIcon::ActivationReason)));

	m_SystemTrayContextMenu.addAction(ui.actionRestore);
	m_SystemTrayContextMenu.addAction(ui.actionSettings);
	m_SystemTrayContextMenu.addAction(ui.actionExit);
	m_SystemTrayContextMenu.insertSeparator(ui.actionExit);
	m_SystemTray.setContextMenu(&m_SystemTrayContextMenu);
	m_SystemTray.show();


	// apply shortcuts
	apply_shortcuts_settings();

	// remove old history
	gc_history();


//	ChatHistory->ClearAll();
//	EventLog->ClearAll();

/*
	QShortcut s(QKeySequence(Qt::CTRL + Qt::Key_P));
	ui.actionConnect->setShortcut(s);
//*/

/*
	QIcon icon(":/lh3/Resources/lh_icon.png");
	QSystemTrayIcon *tray = new QSystemTrayIcon(icon);
	tray->show();
//	tray->showMessage("testing", "message");
//	connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(show()));
	connect(tray, SIGNAL(messageClicked()), this, SLOT(close()));
//*/



/*
	// Select actions in Visitor's tree window
	connect(visitorsTree, SIGNAL(OnVisitorSelect(const uint_t)), this, SLOT(visitorSelected(const uint_t)));
	connect(visitorsTree, SIGNAL(OnVisitorDeselect()), this, SLOT(visitorDeselected()));
//*/


//	updateVisitorActionToolbar();

/*
        for (int i = 0; i < 1000; ++i)
        {
                ChatHistory->DoQuery("INSERT INTO visitor_chat_log (id, date, message) VALUES ('" + QString::number(i) + "', '321', 'Testings...')");
        }
//*/

/*
	QSqlQuery q = ChatHistory->DoSelect("SELECT id, visitor_id, message FROM visitor_chat_log");
	while (q.next())
	{
		int id = q.value(0).toInt();
		int date = q.value(1).toInt();
		QString message = q.value(2).toString();

		int x = 0;
	}
//*/

}

lh3::~lh3()
{
	m_SystemTray.hide();
}


void lh3::connected()
{
	cGlobals::SetConnected();

	setConnected(true);
	update_window_caption();

	widgetStickers->InitFill();
	widgetOfflineMessages->InitFill();

	widgetStickers->OnConnected();
}
void lh3::disconnected()
{
	cGlobals::SetDisconnected();

	CDataBase::ClearShopHash();

	setConnected(false);
	visitors.Cleanup();
	visitorsTree->Cleanup();
	widgetStickers->Cleanup();
	widgetOfflineMessages->Cleanup();

	update_window_caption();

	widgetStickers->OnDisconnected();
}
void lh3::OnConnectionActive()
{
	QMessageBox::critical(this, tr("Login failed", "MSG_LOGIN_FAILED"), tr("Operator with this login is already authorized.", "MSG_ALREADY_AUTHORIZED"), QMessageBox::Ok);
}
void lh3::tryConnectError(QNetworkReply::NetworkError code, const QString &descr, bool auth_failed)
{
	const QString message = (auth_failed) ? tr("Authentication failed. Please try again.", "MSG_AUTH_FAILED") :
		tr("Connection Error: ", "MSG_CONNECTION_ERROR") + descr;

	QMessageBox::critical(this, tr("Error", "MSG_ERROR"), message, QMessageBox::Ok);
}
void lh3::connectionError(QNetworkReply::NetworkError code, const QString &descr, bool auth_failed)
{
	const QString message = (auth_failed) ? tr("Authentication failed. Please try again.", "MSG_AUTH_FAILED") :
		tr("Connection lost: ", "MSG_CONNECTION_LOST") + descr;

	QMessageBox::critical(this, tr("Error", "MSG_ERROR"), message, QMessageBox::Ok);
}

/*
void lh3::respondReceivedXML(const QString &xml_string)
{
	QMutexLocker locker(&mutex);

//	visitors.UpdateFromXML(xml_string);
//	visitorsMDI->UpdateFromXML(xml_string);
}
//*/
void lh3::respondReceivedXML(const CNodeXML &root)
{
	visitors.UpdateFromXML(root);
	visitorsMDI->UpdateFromXML(root);

	cGlobals::UpdateFromXML(root);
	_chat_history.UpdateFromXML(root);
	_event_log.UpdateFromXML(root);

//	ui.dockWidgetStickers->UpdateFromXML(root);
	widgetStickers->UpdateFromXML(root);
	widgetOfflineMessages->UpdateFromXML(root);

/*
	// display operator's name and connected store alias
	QString caption;
	const QString operator_name(cGlobals::GetAssignedOperatorName());

	if (operator_name.length() > 0)
		caption += "[" + cGlobals::GetAssignedOperatorName() + "]";

	if (store_name.length() > 0)
		caption += " at store [" + store_name + "]";
//*/

	update_window_caption();
}
void lh3::openVisitorMDI(const uint_t visitor_id)
{
	QMutexLocker locker(&mutex);

	cVisitor visitor;
	if (visitors.GetVisitor(visitor_id, visitor))
		visitorsMDI->AddVisitor(visitor);
}
void lh3::renameVisitor(const uint_t visitor_id)
{
	cVisitor visitor;
	if (visitors.GetVisitor(visitor_id, visitor))
	{
		cDialogVisitorRename dialog(visitor.GetNameByOperator());
		if (dialog.exec() == QDialog::Accepted)
		{
			const QString name = dialog.GetName();

			visitors.RenameVisitor(visitor_id, name);
		}
	}
}


void lh3::EventNewVisitor()
{
	if (cSettings::IsRaiseNewVisitor())
	{
		if (isHidden())
			show();

		if (isMinimized())
			showMaximized();
	}

	// play sound
	if (cSettings::IsSoundNewVisitor())
		play_sound(cSettings::GetSoundNewVisitorFile());
}
void lh3::EventWaitingVisitor()
{
	if (cSettings::IsRaiseWaitingChat())
	{
		if (isHidden())
			show();

		if (isMinimized())
			showMaximized();
	}

	// play sound
	if (cSettings::IsSoundWaitingChat())
		play_sound(cSettings::GetSoundWaitingChatFile());
}
void lh3::EventAwayVisitor()
{
	// play sound
	if (cSettings::IsSoundAwayVisitor())
		play_sound(cSettings::GetSoundAwayVisitorFile());
}
void lh3::EventNewOfflineMessage()
{
	if (cSettings::IsRaiseNewOfflineMessage())
	{
		if (isHidden())
			show();

		if (isMinimized())
			showMaximized();
	}

	// play sound
	if (cSettings::IsSoundNewOfflineMessage())
		play_sound(cSettings::GetSoundNewOfflineMessageFile());
}
void lh3::EventNewChatMessage()
{
	if (cSettings::IsRaiseNewMessage())
	{
		if (isHidden())
			show();

		if (isMinimized())
			showMaximized();
	}

	// play sound
	if (cSettings::IsSoundNewReplyMessage())
		play_sound(cSettings::GetSoundNewReplyMessageFile());
}

void lh3::SystemTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::Trigger)
	{
		if (!isHidden() && !isMinimized())
		{
			hide();
			return;
		}

		if (isHidden())
			show();

		if (isMinimized())
			showMaximized();
	}
}

void lh3::closeEvent(QCloseEvent *event)
{
	if (ignore_minimaze_on_close == false && cSettings::IsHideOnProgrammClose())
	{
		event->ignore();
//		showMinimized();
		hide();
	}
	else
		event->accept();
}

void lh3::ForceExit()
{
	ignore_minimaze_on_close = true;
	close();
}

void lh3::play_sound(const QString &filename)
{
	if( QSound::isAvailable())
		QSound::play(filename);
}

void lh3::showAbout()
{
	cDialogAbout *dialog = new cDialogAbout;
	dialog->exec();
}


void lh3::Connect()
{
	CDataBase::ClearShopHash();

#ifdef FAST_LOGIN_DEMO_DEMO

	cConnectionParams params;
	params.SetUrl(QUrl(FAST_LOGIN_URL));

	const QString login = "demo";
	const QString password = "fe01ce2a7fbac8fafaed7c982a04e229";
	cGlobals::SetConnectionParams(params, login, password);

	network.Connect(this, login, password, params);

	const QString hash = params.GetShopHash();
	CDataBase::SetShopHash(hash);

	store_name = "Fast Connection";

#else

	cDialogConnect dialog;
	if (dialog.GetConnectionsCount() <= 0)
	{
		QMessageBox::information(this, tr("Information", "MSG_INFORMATION"), 
			tr("Connection not set. Please setup connection first.", "MSG_CONNECTION_NOT_SET"));

		return;
	}
	else
	{
		const int result = dialog.exec();
		if (result > 0)
		{
			cConnectionParams params = dialog.GetConnectionParams();

			const QString login = dialog.GetUsername();
			const QString password = dialog.GetPassword();
			cGlobals::SetConnectionParams(params, login, password);

			network.Connect(this, login, password, params);

			const QString hash = params.GetShopHash();
			CDataBase::SetShopHash(hash);

			store_name = params.GetAlias();
		}
	}

#endif
}
void lh3::Disconnect()
{
	if (DialogAreYouSure(this, tr("Disconnect", "MSG_DISCONNECT")) == 0)
	{
		network.Disconnect();
	}
}

void lh3::update_window_caption()
{
	QString caption;

	if (cGlobals::IsConnected())
	{
		const QString operator_name(cGlobals::GetAssignedOperatorName());

		if (operator_name.length() > 0)
			caption += tr("Operator", "MSG_OPERATOR") + " [" + cGlobals::GetAssignedOperatorName() + "]";

		if (store_name.length() > 0)
			caption += QString(" ") + tr("at store", "MSG_AT_STORE") + " [" + store_name + "]";
	}

	QString text("Live Help v3.0");
	if (caption.size() > 0)
		text += " - " + caption;

	setWindowTitle(text);
}


void lh3::execConfig()
{
	if (isHidden())
		show();

	if (isMinimized())
		showMaximized();

	cDialogConfig dialog;
	connect(&dialog, SIGNAL(OnSettingsApplied()), this, SLOT(OnSettingsApplied()));
	connect(&dialog, SIGNAL(OnLanguageChanged(const QString&)), this, SLOT(OnLanguageChanged(const QString&)));
	dialog.exec();
	
}

void lh3::OnLanguageChanged(const QString &filename)
{
	CTranslator::LoadLanguage(filename);
	ApplyLanguageLabels();
}

void lh3::OnSettingsApplied()
{
	apply_shortcuts_settings();
	gc_history();
	ApplyLanguageLabels();
}

void lh3::apply_shortcuts_settings()
{
	QAction *actions[ShortcutsCount] = {ui.actionConnect, ui.actionDisconnect, ui.actionVisitors, ui.actionOffline_Messages, ui.actionStickers,
		ui.actionWindows, ui.actionSettings, ui.actionTile, ui.actionCascade, ui.actionClose_Active, ui.actionClose_All};

	for (int i = 0; i < ShortcutsCount; ++i)
	{
		QAction *action = actions[i];
		action->setShortcut(cSettings::GetShortcut(static_cast<ShortcutType>(i)));
	}
}

void lh3::gc_history()
{
	const uint_t days = cSettings::GetHistoryKeepingDays();
	if (days == 0)
		return;

	const uint_t date = QDateTime::currentDateTime().toTime_t() - (days * 86400);

	ChatHistory->ClearAll(date);
	EventLog->ClearAll(date);
}

void lh3::ApplyLanguageLabels()
{
	ui.retranslateUi(this);

	visitorsTree->ApplyLanguageLabels();
	widgetOfflineMessages->ApplyLanguageLabels();
	widgetStickers->ApplyLanguageLabels();
	visitorsMDI->ApplyLanguageLabels();

	update_window_caption();
}