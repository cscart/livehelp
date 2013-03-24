#pragma once

#include <map>
#include <list>
#include <memory>
#include "ui_WidgetDockOfflineMessages.h"
#include "OfflineMessages.h"
#include "DialogOfflineMessages.h"
#include "Translator.h"

using std::map;
using std::list;
using std::auto_ptr;

class CWidgetOfflineMessages : public QWidget, public ILanguageUI
{
	Q_OBJECT

private:
	enum Columns
	{
		ColumnSubject,
		ColumnId,
		ColumnVisitor,
		ColumnDate
	};

	typedef map<uint_t, COfflineMessageRecord> OfflineMessagesArray;

	Ui::wDockOfflineMessages ui;

	OfflineMessagesArray messages;
	mutable list<COfflineMessageRecord> replyMessages;
	mutable list<COfflineMessageRecord> deletedMessages;
	COfflineMessages dbMessages;


	void fill_item(QTreeWidgetItem *const item, const COfflineMessageRecord &record);
	void display_item(QTreeWidgetItem *item);

public:
	CWidgetOfflineMessages();
	~CWidgetOfflineMessages();

	void UpdateFromXML(const CNodeXML &root);

	void OnRecordAdd(const COfflineMessageRecord&);
	void OnRecordUpdate(const COfflineMessageRecord&);
	void OnRecordDelete(const uint_t&);

	void SetUnread(const COfflineMessageRecord &record, const bool value=true);

	void InitFill();
	void Cleanup();

	// override
	virtual void ApplyLanguageLabels() { ui.retranslateUi(this); }

public slots:;

	void GetMessagesDOM(QDomElement &root) const;

	void itemSelectionChanged();
	void itemDoubleClicked(QTreeWidgetItem *item, int) { display_item(item); }

	void ViewRecord();
//	void AddRecord();
	void DeleteRecord();



signals:;
	void EventNewOfflineMessage();

};
