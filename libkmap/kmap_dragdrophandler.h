/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-03-18
 * @brief  Drag-and-drop handler for KMap
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef KMAP_DRAGDROPHANDLER_H
#define KMAP_DRAGDROPHANDLER_H

// Qt includes

#include <QAbstractItemModel>

// local includes

#include "kmap_primitives.h"
#include "libkmap_export.h"

class QDropEvent;

namespace KMapIface
{

class KMAP_EXPORT DragDropHandler : public QObject
{
    Q_OBJECT

public:
    DragDropHandler(QObject* const parent = 0);
    virtual ~DragDropHandler();

    virtual Qt::DropAction accepts(const QDropEvent* e) = 0;
    virtual bool dropEvent(const QDropEvent* e, const WMWGeoCoordinate& dropCoordinates, QList<QPersistentModelIndex>* const droppedIndices) = 0;
    virtual QMimeData* createMimeData(const QList<QPersistentModelIndex>& modelIndices) = 0;
};

} /* namespace KMapIface */

#endif /* KMAP_DRAGDROPHANDLER_H */
