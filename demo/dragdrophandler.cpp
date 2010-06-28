/* ============================================================
 *
 * Date        : 2010-03-18
 * Description : Drag-and-drop handler for WorldMapWidget2 used in the demo
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

#include "dragdrophandler.moc"

// Qt includes

#include <QDropEvent>

// local includes

#include "mytreewidget.h"

DemoDragDropHandler::DemoDragDropHandler(QAbstractItemModel* const pModel, QObject* const parent)
                   : DragDropHandler(parent), model(pModel)
{
}

DemoDragDropHandler::~DemoDragDropHandler()
{
}

Qt::DropAction DemoDragDropHandler::accepts(const QDropEvent* /*e*/)
{
    return Qt::CopyAction;
}

bool DemoDragDropHandler::dropEvent(const QDropEvent* e, const WMW2::WMWGeoCoordinate& dropCoordinates, QList<QPersistentModelIndex>* const droppedIndices)
{
    const MyDragData* const mimeData = qobject_cast<const MyDragData*>(e->mimeData());
    if (!mimeData)
        return false;

    kDebug()<<mimeData->draggedIndices.count();

    for (int i=0; i<mimeData->draggedIndices.count(); ++i)
    {
        const QPersistentModelIndex itemIndex = mimeData->draggedIndices.at(i);
        if (!itemIndex.isValid())
            continue;

        model->setData(itemIndex, QVariant::fromValue(dropCoordinates), RoleCoordinates);
    }

    // let the WorldMapWidget2 know which markers were dropped:
    if (droppedIndices)
    {
        *droppedIndices = mimeData->draggedIndices;
    }

    return true;
}

QMimeData* DemoDragDropHandler::createMimeData(const QList<QPersistentModelIndex>& /*modelIndices*/)
{
    return 0;
}

