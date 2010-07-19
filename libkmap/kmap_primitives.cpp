/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  Primitive datatypes for KMap
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
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

#include "kmap_primitives.moc"

namespace KMapIface
{

/**
 * @class WMWModelHelper
 * @brief Helper class to access data in models.
 *
 * @c WMWModelHelper is used to access data held in models, which is not suitable for transfer using the
 * the Qt-style API, like coordinates or custom sized thumbnails.
 *
 * The basic functions which have to be implemented are:
 * @li model(): Returns a pointer to the model
 * @li selectionModel(): Returns a pointer to the selection model. It may return a null-pointer
 *     if no selection model is used.
 * @li itemCoordinates(): Returns the coordinates for a given item index, if it has any.
 *
 * For ungrouped models, the following functions should also be implemented:
 * @li itemIcon(): Returns an icon for an index, and an offset to the 'center' of the item.
 * @li modelFlags(): Returns flags for the model.
 * @li itemFlags(): Returns flags for individual items.
 * @li snapItemsTo(): Grouped items have been moved and should snap to an index.
 *
 * For grouped models which are accessed by @c MarkerModel, the following functions should be implemented:
 * @li bestRepresentativeIndexFromList(): Find the item that should represent a group of items.
 * @li pixmapFromRepresentativeIndex(): Find a thumbnail for an item.
 */


WMWModelHelper::WMWModelHelper(QObject* const parent)
              : QObject(parent)
{
}

WMWModelHelper::~WMWModelHelper()
{
}

void WMWModelHelper::snapItemsTo(const QModelIndex& targetIndex, const QList<QPersistentModelIndex>& snappedIndices)
{
    QList<QModelIndex> result;
    for (int i=0; i<snappedIndices.count(); ++i)
    {
        result << snappedIndices.at(i);
    }
    snapItemsTo(targetIndex, result);
}

QPersistentModelIndex WMWModelHelper::bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list, const int /*sortKey*/)
{
    // this is only a stub to provide some default implementation
    if (list.isEmpty())
        return QPersistentModelIndex();

    return list.first();
}

QPixmap WMWModelHelper::itemIcon(const QModelIndex& /*index*/, QPoint* const /*offset*/) const
{
    return QPixmap();
}

WMWModelHelper::Flags WMWModelHelper::modelFlags() const
{
    return Flags();
}

WMWModelHelper::Flags WMWModelHelper::itemFlags(const QModelIndex& /*index*/) const
{
    return Flags();
}

void WMWModelHelper::snapItemsTo(const QModelIndex& /*targetIndex*/, const QList<QModelIndex>& /*snappedIndices*/)
{
}

QPixmap WMWModelHelper::pixmapFromRepresentativeIndex(const QPersistentModelIndex& /*index*/, const QSize& /*size*/)
{
    return QPixmap();
}

void WMWModelHelper::onIndicesClicked(const QList<QPersistentModelIndex>& clickedIndices)
{
    Q_UNUSED(clickedIndices);
}

void WMWModelHelper::onIndicesMoved(const QList<QPersistentModelIndex>& movedIndices, const WMWGeoCoordinate& targetCoordinates, const QPersistentModelIndex& targetSnapIndex)
{
    Q_UNUSED(movedIndices);
    Q_UNUSED(targetCoordinates);
    Q_UNUSED(targetSnapIndex);
}

} /* namespace KMapIface */
