#include "stdafx.h"
#include "WidgetVisitorsTrayItem.h"

cWidgetVisitorsTrayItem::cWidgetVisitorsTrayItem(const cVisitor &_visitor)
:visitor(_visitor), is_down(false)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	Update(visitor);
}

void cWidgetVisitorsTrayItem::Update(const cVisitor &_visitor)
{
	visitor = _visitor;

	ui.pushButton->setIcon(visitor.GetStatusIcon());

	const QString title = visitor.GetStatusDescr() + " - [" + visitor.GetCombinedName() + "]";
	ui.pushButton->setText(title);
}

void cWidgetVisitorsTrayItem::SetLoggedOff()
{
	ui.pushButton->setIcon(cVisitor::GetNoUserIcon());

	const QString title = cVisitor::GetNoUserDesrc() + " - [" + visitor.GetCombinedName() + "]";
	ui.pushButton->setText(title);
}

void cWidgetVisitorsTrayItem::SetSelected(bool res)
{
	is_down = res;
	ui.pushButton->setChecked(res);
}


void cWidgetVisitorsTrayItem::selectButtonClicked()
{
	ui.pushButton->setChecked(is_down);

	OnWindowSelect(visitor);
}