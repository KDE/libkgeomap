/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-03-18
 * @brief  Drag-and-drop handler for KMap used in the demo
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

bool DemoDragDropHandler::dropEvent(const QDropEvent* e, const KMap::GeoCoordinates& dropCoordinates)
{
    const MyDragData* const mimeData = qobject_cast<const MyDragData*>(e->mimeData());
    if (!mimeData)
        return false;

    kDebug() << mimeData->draggedIndices.count();

    for (int i=0; i<mimeData->draggedIndices.count(); ++i)
    {
        const QPersistentModelIndex itemIndex = mimeData->draggedIndices.at(i);
        if (!itemIndex.isValid())
            continue;

        model->setData(itemIndex, QVariant::fromValue(dropCoordinates), RoleCoordinates);
    }

    // TODO: tell the main window about this so it can start an altitude lookup

    return true;
}

QMimeData* DemoDragDropHandler::createMimeData(const QList<QPersistentModelIndex>& /*modelIndices*/)
{
    return 0;
}
