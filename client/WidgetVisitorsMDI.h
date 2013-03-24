#pragma once

#include "ui_WidgetVisitorsMDI.h"
#include "WidgetVisitorDoc.h"
#include "Message.h"
#include "Translator.h"

class cWidgetVisitorsMDI : public QWidget, public ILanguageUI
{
	Q_OBJECT

private:
	Ui::wVisitorsMDI ui;

//	cWidgetVisitorDoc* find_visitor_doc(const cVisitor &visitor, QMdiSubWindow **sub_window=NULL);
	cWidgetVisitorDoc* find_visitor_doc(const uint_t visitor_id, QMdiSubWindow **sub_window);
	void refresh_sub_window(const cVisitor &visitor, QMdiSubWindow *subWindow);

//	void UpdateChatMessagesFromXML(const QString &string);
	void UpdateChatMessagesFromXML(const CNodeXML &root);

public:
	cWidgetVisitorsMDI();
	~cWidgetVisitorsMDI();

	// override
	virtual void ApplyLanguageLabels();

private slots:;

	void destroyVisitorDoc(QObject *object);

	void subWindowActivated(QMdiSubWindow *subWindow);


public slots:;

	void AddVisitor(const cVisitor &visitor);
	void UpdateVisitor(const cVisitor &visitor, const bool);
	void UpdateVisitor(const cVisitor &old_visitor, const cVisitor &new_visitor);
	void RemoveVisitor(const cVisitor &visitor);

	void SelectWindow(const cVisitor& visitor);

	void cascadeSubWindows() { ui.mdiArea->cascadeSubWindows(); }
	void tileSubWindows() { ui.mdiArea->tileSubWindows(); }
	void closeAll() { ui.mdiArea->closeAllSubWindows(); }
	void closeActive() { ui.mdiArea->closeActiveSubWindow(); }

	// return 'true' if at least one chat window opened
	void HasOpenedChat(bool &result);


	// Return DOM of visitor's messages
	void GetVisitorParamsDOM(QDomElement &root) const;

//	void UpdateFromXML(const QString &string);
	void UpdateFromXML(const CNodeXML &root);

signals:;

	void OnOpenWindow(const cVisitor&);
	void OnUpdateWindow(const cVisitor&, const cVisitor&);
	void OnCloseWindow(const uint_t);

	void OnWindowActivated(const uint_t);


	void OnErrorParseXML();


	void EventNewChatMessage();
};
