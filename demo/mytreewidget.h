/* ============================================================
 *
 * Date        : 2010-03-06
 * Description : sub class of QTreeWidget for drag-and-drop support
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef MYTREEWIDGET_H
#define MYTREEWIDGET_H

// Qt includes

#include <QTreeWidget>
#include <QPersistentModelIndex>

class MyTreeWidgetPrivate;
class QMouseEvent;

Q_DECLARE_METATYPE(QTreeWidgetItem*)
Q_DECLARE_METATYPE(QPersistentModelIndex)

const int RoleMyData = Qt::UserRole+0;
const int RoleCoordinates = Qt::UserRole+1;

class MyTreeWidget : public QTreeWidget
{
Q_OBJECT

public:
    MyTreeWidget(QWidget* const parent = 0);
    ~MyTreeWidget();

protected:
//     void mousePressEvent(QMouseEvent* event);
//     void mouseMoveEvent(QMouseEvent* event);
    void startDrag(Qt::DropActions supportedActions);
    virtual QMimeData * mimeData ( const QList<QTreeWidgetItem *> items ) const;

private:
    MyTreeWidgetPrivate* const d;
};

#endif /* MYTREEWIDGET_H */

