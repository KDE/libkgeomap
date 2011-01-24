/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-07-17
 * @brief  A marker tiler operating on item models
 *
 * @author Copyright (C) 2010, 2011 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
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

// Qt includes

#include <QtGui/QItemSelection>

// local includes

#include "abstractmarkertiler.h"


namespace KMap
{

class ModelHelper;

class KMAP_EXPORT ItemMarkerTiler : public AbstractMarkerTiler
{
    Q_OBJECT

private:

    class MyTile;

public:

    explicit ItemMarkerTiler(ModelHelper* const modelHelper, QObject* const parent = 0);
    virtual ~ItemMarkerTiler();

    virtual Flags tilerFlags() const;
    virtual Tile* tileNew();
    virtual void tileDeleteInternal(Tile* const tile);
    virtual void prepareTiles(const GeoCoordinates& upperLeft, const GeoCoordinates& lowerRight, int level);
    virtual void regenerateTiles();
    virtual Tile* getTile(const TileIndex& tileIndex, const bool stopIfEmpty = false);
    virtual int getTileMarkerCount(const TileIndex& tileIndex);
    virtual int getTileSelectedCount(const TileIndex& tileIndex);

    virtual QVariant getTileRepresentativeMarker(const TileIndex& tileIndex, const int sortKey);
    virtual QVariant bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey);
    virtual QPixmap pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size);
    virtual bool indicesEqual(const QVariant& a, const QVariant& b) const;
    virtual KMapGroupState getTileGroupState(const TileIndex& tileIndex);
    virtual KMapGroupState getGlobalGroupState();

    virtual void onIndicesClicked(const TileIndex::List& tileIndicesList, const QVariant& representativeIndex,
                                  const KMapGroupState& groupSelectionState, const MouseModes currentMouseMode);
    virtual void onIndicesMoved(const TileIndex::List& tileIndicesList, const GeoCoordinates& targetCoordinates,
                                const QPersistentModelIndex& targetSnapIndex);

    void setMarkerModelHelper(ModelHelper* const modelHelper);
    void removeMarkerIndexFromGrid(const QModelIndex& markerIndex, const bool ignoreSelection = false);
    void addMarkerIndexToGrid(const QPersistentModelIndex& markerIndex);

    void setActive(const bool state);

private Q_SLOTS:

    void slotSourceModelRowsInserted(const QModelIndex& parentIndex, int start, int end);
    void slotSourceModelRowsAboutToBeRemoved(const QModelIndex& parentIndex, int start, int end);
    void slotSourceModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void slotSourceModelReset();
    void slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void slotThumbnailAvailableForIndex(const QPersistentModelIndex& index, const QPixmap& pixmap);
    void slotSourceModelLayoutChanged();

private:

    QList<QPersistentModelIndex> getTileMarkerIndices(const TileIndex& tileIndex);

    class ItemMarkerTilerPrivate;
    ItemMarkerTilerPrivate* const d;
};

} // namespace KMap

#endif /* ITEMMARKERTILER_H */
