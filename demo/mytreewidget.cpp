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

#include "mytreewidget.moc"

// Qt includes

#include <QApplication>
#include <QMouseEvent>

// local includes

#include <worldmapwidget2_primitives.h>

class MyTreeWidgetPrivate
{
public:
    MyTreeWidgetPrivate()
    : dragStartPos()
    {
    }

    QPoint dragStartPos;
};

MyTreeWidget::MyTreeWidget(QWidget* const parent)
: QTreeWidget(parent), d(new MyTreeWidgetPrivate())
{
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);
}

MyTreeWidget::~MyTreeWidget()
{
    delete d;
}

void MyTreeWidget::startDrag(Qt::DropActions supportedActions)
{
    QMimeData* const dragMimeData = mimeData(selectedItems());

    QDrag* const drag = new QDrag(this);
    drag->setMimeData(dragMimeData);
    drag->start(Qt::CopyAction);
}

QMimeData* MyTreeWidget::mimeData(const QList<QTreeWidgetItem*> itemsToDrag) const
{
    WMW2::WMWDragData* const mimeData = new WMW2::WMWDragData;

    QString dragText;
    for (int i=0; i<itemsToDrag.count(); ++i)
    {
        QTreeWidgetItem* const treeItem = itemsToDrag.at(i);

        if (!dragText.isEmpty())
            dragText+=", ";
        dragText+=treeItem->text(0);

        const QPersistentModelIndex itemIndex = treeItem->data(0, RoleMyData).value<QPersistentModelIndex>();
        mimeData->itemIndices << itemIndex;
        mimeData->itemCount++;
    }
    mimeData->setText(dragText);

    return mimeData;

//     QDrag* const drag = new QDrag(this);
//     drag->setMimeData(mimeData);
//     drag->start(Qt::CopyAction);
// 
//     return QTreeWidget::mimeData(items);
}