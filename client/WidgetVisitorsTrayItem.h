#pragma once

#include "ui_WidgetVisitorsTrayItem.h"
#include "Visitor.h"

class cWidgetVisitorsTrayItem : public QWidget
{
	Q_OBJECT

private:
	Ui::FormVisitorsTrayItem ui;

	cVisitor visitor;
	bool is_down;

public:
	cWidgetVisitorsTrayItem(const cVisitor &_visitor);
	~cWidgetVisitorsTrayItem() {}

	uint_t GetVisitorId() const { return visitor.GetId(); }

	void Update(const cVisitor &_visitor);
	void SetLoggedOff();

	void SetSelected(bool res=true);


private slots:;

	void selectButtonClicked();


signals:;

	void OnWindowSelect(const cVisitor&);

};
