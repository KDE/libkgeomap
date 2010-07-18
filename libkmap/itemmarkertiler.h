/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-07-17
 * @brief  A marker tiler operating on item models
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

#ifndef ITEMMARKERTILER_H
#define ITEMMARKERTILER_H

// local includes

#include "abstractmarkertiler.h"

namespace KMapIface
{

class ItemMarkerTilerPrivate;

class KMAP_EXPORT ItemMarkerTiler : public AbstractMarkerTiler
{
Q_OBJECT

public:

    ItemMarkerTiler(WMWModelHelper* const modelHelper, QObject* const parent = 0);
    ~ItemMarkerTiler();

    virtual bool isItemModelBased() const;
    virtual QItemSelectionModel* getSelectionModel() const;
    virtual QAbstractItemModel* getModel() const;
    virtual QList<QPersistentModelIndex> getTileMarkerIndices(const TileIndex& tileIndex);

    virtual void regenerateTiles();
    virtual Tile* getTile(const TileIndex& tileIndex, const bool stopIfEmpty = false);
    virtual int getTileMarkerCount(const TileIndex& tileIndex);
    virtual int getTileSelectedCount(const TileIndex& tileIndex);

    virtual QVariant getTileRepresentativeMarker(const TileIndex& tileIndex, const int sortKey);
    virtual QVariant bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey);
    virtual QPixmap pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size);
    virtual bool indicesEqual(const QVariant& a, const QVariant& b) const;
    virtual WMWSelectionState getTileSelectedState(const TileIndex& tileIndex);

    virtual void onIndicesClicked(const TileIndex::List& tileIndicesList);
    virtual void onIndicesMoved(const TileIndex::List& tileIndicesList, const WMWGeoCoordinate& targetCoordinates, const QPersistentModelIndex& targetSnapIndex);

    void setMarkerModelHelper(WMWModelHelper* const modelHelper);
    void removeMarkerIndexFromGrid(const QModelIndex& markerIndex, const bool ignoreSelection = false);
    void addMarkerIndexToGrid(const QPersistentModelIndex& markerIndex);
    

private Q_SLOTS:

    void slotSourceModelRowsInserted(const QModelIndex& parentIndex, int start, int end);
    void slotSourceModelRowsAboutToBeRemoved(const QModelIndex& parentIndex, int start, int end);
    void slotSourceModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void slotSourceModelReset();
    void slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void slotThumbnailAvailableForIndex(const QPersistentModelIndex& index, const QPixmap& pixmap);

private:

    ItemMarkerTilerPrivate* const d;
};

} // KMapIface

#endif /* ITEMMARKERTILER_H */
