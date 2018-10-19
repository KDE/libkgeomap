/** ===========================================================
 *
 * This file is a part of KDE project
 *
 *
 * @date   2010-03-06
 * @brief  sub class of QTreeWidget for drag-and-drop support
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef MYTREEWIDGET_H
#define MYTREEWIDGET_H

// Qt includes

#include <QTreeWidget>
#include <QPersistentModelIndex>

// local includes

#include "myimageitem.h"

class QMouseEvent;

Q_DECLARE_METATYPE(QTreeWidgetItem*)

class MyTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:

    explicit MyTreeWidget(QWidget* const parent = nullptr);
    ~MyTreeWidget() override;

protected:

//     void mousePressEvent(QMouseEvent* event);
//     void mouseMoveEvent(QMouseEvent* event);
    void startDrag(Qt::DropActions supportedActions) override;
    QMimeData* mimeData(const QList<QTreeWidgetItem*> items) const override;
    virtual QMimeData* mimeData(const QModelIndexList items)         const;

private:

    class Private;
    Private* const d;
};

#endif /* MYTREEWIDGET_H */
