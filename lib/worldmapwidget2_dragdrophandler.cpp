/* ============================================================
 *
 * Date        : 2010-03-18
 * Description : Drag-and-drop handler for WorldMapWidget2
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

#include "worldmapwidget2_dragdrophandler.moc"

// Qt includes

#include <QAbstractItemModel>

// local includes

#include "worldmapwidget2_primitives.h"
#include "worldmapwidget2_export.h"

namespace WMW2 {

DragDropHandler::DragDropHandler(QObject* const parent)
: QObject(parent)
{
}

DragDropHandler::~DragDropHandler()
{
}

} /* WMW2 */