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

#pragma once

#include "WidgetVisitorsTrayItem.h"

class LayoutVisitorsTray : public QLayout
{
	Q_OBJECT // custom

public:
  LayoutVisitorsTray(QWidget *parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
  LayoutVisitorsTray(int margin = -1, int hSpacing = -1, int vSpacing = -1);
	~LayoutVisitorsTray() {}


protected:
  void addItem(QLayoutItem *item);

  int horizontalSpacing() const;
  int verticalSpacing() const;
  Qt::Orientations expandingDirections() const;
  bool hasHeightForWidth() const;
  int heightForWidth(int) const;
  int count() const;
  QLayoutItem *itemAt(int index) const;
  QSize minimumSize() const;
  void setGeometry(const QRect &rect);
  QSize sizeHint() const;
  QLayoutItem *takeAt(int index);

private:
  int doLayout(const QRect &rect, bool testOnly) const;
  int smartSpacing(QStyle::PixelMetric pm) const;

  QList<QLayoutItem *> itemList;
  int m_hSpace;
  int m_vSpace;


// custom

private:
	typedef QMap<uint_t, QWidget*> VisitorList;

	VisitorList visitors;

	void uncheck_all();

private slots:;

	void SelectWindow(const cVisitor& visitor);

public slots:;

	void OpenWindow(const cVisitor &visitor);
	void UpdateWindow(const cVisitor &old_visitor, const cVisitor &new_visitor);
	void CloseWindow(const uint_t id);
	void RemoveVisitor(const cVisitor &visitor);

	void WindowActivated(const uint_t id);



signals:;

	void OnWindowSelect(const cVisitor&);

};

