#pragma once

#include <QPainter>
#include <map>
#include <list>
#include <memory>
#include "ui_WidgetDockStickers.h"
#include "common.h"
#include "Globals.h"
#include "Stickers.h"
#include "defines.h"
#include "DialogStickers.h"
#include "Translator.h"

#include "BehaviourUpdating.h"

using std::map;
using std::list;
using std::auto_ptr;

class CWidgetStickers : public QWidget, public ILanguageUI, protected IBehaviourUpdating
{
	Q_OBJECT

private:
	enum Columns
	{
		ColumnTitle,
		ColumnId,
		ColumnOperator,
		ColumnDate
	};

	typedef map<uint_t, CStickerRecord> StickersArray;

	Ui::wDockStickers ui;

	StickersArray stickers;
	mutable list<CStickerRecord> addedStickers;
	mutable list<CStickerRecord> deletedStickers;
	CStickers dbStickers;

	void fill_item(QTreeWidgetItem *const item, const CStickerRecord &record);
	void display_item(QTreeWidgetItem *item);
	bool is_owner(const QTreeWidgetItem *const item);

protected:
	// overload
	virtual void OnEnable(const bool enabled);

public:
	CWidgetStickers();
	~CWidgetStickers();

	void UpdateFromXML(const CNodeXML &root);


	void OnRecordAdd(const CStickerRecord&);
	void OnRecordUpdate(const CStickerRecord&);
	void OnRecordDelete(const uint_t&);

//	void SetUnread(const CStickerRecord &record, const bool value=true);

	void InitFill();
	void Cleanup();

	// override
	virtual void ApplyLanguageLabels() { ui.retranslateUi(this); }

	// connection events
	void OnConnected();
	void OnDisconnected();

public slots:;

	void GetStickersDOM(QDomElement &root) const;

	void itemSelectionChanged();
	void itemDoubleClicked(QTreeWidgetItem *item, int) { display_item(item); }

	void ViewRecord();
	void AddRecord();
	void DeleteRecord();


};
