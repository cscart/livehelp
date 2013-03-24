/****************************************************************************
 **
 ** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** Commercial Usage
 ** Licensees holding valid Qt Commercial licenses may use this file in
 ** accordance with the Qt Commercial License Agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and Nokia.
 **
 ** GNU Lesser General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU Lesser
 ** General Public License version 2.1 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.LGPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU Lesser General Public License version 2.1 requirements
 ** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** In addition, as a special exception, Nokia gives you certain
 ** additional rights. These rights are described in the Nokia Qt LGPL
 ** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
 ** package.
 **
 ** GNU General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU
 ** General Public License version 3.0 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU General Public License version 3.0 requirements will be
 ** met: http://www.gnu.org/copyleft/gpl.html.
 **
 ** If you are unsure which license is appropriate for your use, please
 ** contact the sales department at http://www.qtsoftware.com/contact.
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include "stdafx.h"
#include "LayoutVisitorsTray.h"


LayoutVisitorsTray::LayoutVisitorsTray(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

LayoutVisitorsTray::LayoutVisitorsTray(int margin, int hSpacing, int vSpacing)
    : m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}


void LayoutVisitorsTray::addItem(QLayoutItem *item)
{
    itemList.append(item);
}

int LayoutVisitorsTray::horizontalSpacing() const
{
    if (m_hSpace >= 0) {
        return m_hSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}

int LayoutVisitorsTray::verticalSpacing() const
{
    if (m_vSpace >= 0) {
        return m_vSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

int LayoutVisitorsTray::count() const
{
    return itemList.size();
}

QLayoutItem *LayoutVisitorsTray::itemAt(int index) const
{
    return itemList.value(index);
}

QLayoutItem *LayoutVisitorsTray::takeAt(int index)
{
    if (index >= 0 && index < itemList.size())
        return itemList.takeAt(index);
    else
        return 0;
}

Qt::Orientations LayoutVisitorsTray::expandingDirections() const
{
    return 0;
}

bool LayoutVisitorsTray::hasHeightForWidth() const
{
    return true;
}

int LayoutVisitorsTray::heightForWidth(int width) const
{
    int height = doLayout(QRect(0, 0, width, 0), true);
    return height;
}

void LayoutVisitorsTray::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize LayoutVisitorsTray::sizeHint() const
{
    return minimumSize();
}

QSize LayoutVisitorsTray::minimumSize() const
{
    QSize size;
    QLayoutItem *item;
    foreach (item, itemList)
        size = size.expandedTo(item->minimumSize());

    size += QSize(2*margin(), 2*margin());
    return size;
}

int LayoutVisitorsTray::doLayout(const QRect &rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    QLayoutItem *item;
    foreach (item, itemList) {
        QWidget *wid = item->widget();
        int spaceX = horizontalSpacing();
        if (spaceX == -1)
            spaceX = wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        int spaceY = verticalSpacing();
        if (spaceY == -1)
            spaceY = wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);

        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        if (!testOnly)
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}

int LayoutVisitorsTray::smartSpacing(QStyle::PixelMetric pm) const
{
  QObject *parent = this->parent();
  if (!parent) {
      return -1;
  } else if (parent->isWidgetType()) {
      QWidget *pw = static_cast<QWidget *>(parent);
      return pw->style()->pixelMetric(pm, 0, pw);
  } else {
      return static_cast<QLayout *>(parent)->spacing();
  }
}


// custom area

void LayoutVisitorsTray::OpenWindow(const cVisitor &visitor)
{
	cWidgetVisitorsTrayItem *widget = new cWidgetVisitorsTrayItem(visitor);
	connect(widget, SIGNAL(OnWindowSelect(const cVisitor&)), this, SLOT(SelectWindow(const cVisitor&)));
	addWidget(widget);

	VisitorList::const_iterator it = visitors.find(visitor.GetId());
	if (it == visitors.end())
		visitors[visitor.GetId()] = widget;
}

void LayoutVisitorsTray::UpdateWindow(const cVisitor &old_visitor, const cVisitor &new_visitor)
{
	VisitorList::const_iterator it = visitors.find(old_visitor.GetId());
	if (it == visitors.end())
		return;

	cWidgetVisitorsTrayItem *widget = static_cast<cWidgetVisitorsTrayItem*>(*it);
	widget->Update(new_visitor);

	update();
}

void LayoutVisitorsTray::CloseWindow(const uint_t id)
{
	VisitorList::const_iterator it = visitors.find(id);
	if (it == visitors.end())
		return;

	QWidget *widget = *it;

	removeWidget(widget);
	visitors.remove(id);

	delete widget;
}

void LayoutVisitorsTray::RemoveVisitor(const cVisitor &visitor)
{
	VisitorList::const_iterator it = visitors.find(visitor.GetId());
	if (it == visitors.end())
		return;

	cWidgetVisitorsTrayItem *widget = static_cast<cWidgetVisitorsTrayItem*>(*it);
	widget->SetLoggedOff();

	update();
}

void LayoutVisitorsTray::WindowActivated(const uint_t id)
{
	uncheck_all();
	VisitorList::const_iterator it = visitors.find(id);
	if (it == visitors.end())
		return;

	cWidgetVisitorsTrayItem *widget = static_cast<cWidgetVisitorsTrayItem*>(*it);
	widget->SetSelected();
}

void LayoutVisitorsTray::uncheck_all()
{
	for (VisitorList::iterator it = visitors.begin(); it != visitors.end(); ++it)
	{
		cWidgetVisitorsTrayItem *widget = static_cast<cWidgetVisitorsTrayItem*>(*it);
		widget->SetSelected(false);
	}
}

void LayoutVisitorsTray::SelectWindow(const cVisitor& visitor)
{
	WindowActivated(visitor.GetId());
	OnWindowSelect(visitor);
}