#pragma once

#include "ui_WidgetVisitorsTree.h"
#include "Visitor.h"
#include "Translator.h"

class cWidgetVisitorsTree : public QWidget, public ILanguageUI
{
	Q_OBJECT

private:
	enum Columns
	{
		Name,
		Id,
		Count 
	};

private:
	Ui::wVisitorsTree ui;

	QMutex mutex;

	int get_index(cVisitor::Status status);
	void refresh_section(cVisitor::Status status);

	void cleanup_section(cVisitor::Status status);

	uint_t get_selected_id() const;

public:
	cWidgetVisitorsTree();
	~cWidgetVisitorsTree() {}

	// override
	virtual void ApplyLanguageLabels();

public slots:;

	void AddVisitor(const cVisitor &visitor, const bool expand_section=false);
	void UpdateVisitor(const cVisitor &old_visitor, const cVisitor &new_visitor);
	bool RemoveVisitor(const cVisitor &visitor);

	void Cleanup();


private slots:;
	void itemDoubleClicked(QTreeWidgetItem *item, int column);
	void showPopup(const QPoint &pos);
//	void itemSelectionChanged();

	void openVisitorAction();
	void renameVisitorAction();

signals:;

	void OnVisitorOpen(const uint_t);

	void OnVisitorRename(const uint_t);

/*
	void OnVisitorSelect(const uint_t);
	void OnVisitorDeselect();
//*/
};
