/** ===========================================================
 * @file
 *
 * This file is a part of KDE project
 *
 *
 * @date   2010-07-17
 * @brief  A marker tiler operating on item models
 *
 * @author Copyright (C) 2010, 2011 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010-2015 by Gilles Caulier
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

#ifndef KGEOMAP_ITEMMARKERTILER_H
#define KGEOMAP_ITEMMARKERTILER_H

// Qt includes

#include <QtCore/QItemSelection>

// local includes

#include "abstractmarkertiler.h"

namespace KGeoMap
{

class ModelHelper;

class KGEOMAP_EXPORT ItemMarkerTiler : public AbstractMarkerTiler
{
    Q_OBJECT

public:

    explicit ItemMarkerTiler(ModelHelper* const modelHelper, QObject* const parent = nullptr);
    ~ItemMarkerTiler() override;

    Flags tilerFlags() const override;
    Tile* tileNew() override;
    void tileDeleteInternal(Tile* const tile) override;
    void prepareTiles(const GeoCoordinates& upperLeft, const GeoCoordinates& lowerRight, int level) override;
    void regenerateTiles() override;
    Tile* getTile(const TileIndex& tileIndex, const bool stopIfEmpty = false) override;
    int getTileMarkerCount(const TileIndex& tileIndex) override;
    int getTileSelectedCount(const TileIndex& tileIndex) override;

    QVariant getTileRepresentativeMarker(const TileIndex& tileIndex, const int sortKey) override;
    QVariant bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey) override;
    QPixmap pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size) override;
    bool indicesEqual(const QVariant& a, const QVariant& b) const override;
    GroupState getTileGroupState(const TileIndex& tileIndex) override;
    GroupState getGlobalGroupState() override;

    void onIndicesClicked(const ClickInfo& clickInfo) override;
    void onIndicesMoved(const TileIndex::List& tileIndicesList, const GeoCoordinates& targetCoordinates,
                                const QPersistentModelIndex& targetSnapIndex) override;

    void setMarkerModelHelper(ModelHelper* const modelHelper);
    void removeMarkerIndexFromGrid(const QModelIndex& markerIndex, const bool ignoreSelection = false);
    void addMarkerIndexToGrid(const QPersistentModelIndex& markerIndex);

    void setActive(const bool state) override;

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

private:

    class MyTile;

    class Private;
    Private* const d;
};

} // namespace KGeoMap

#endif // KGEOMAP_ITEMMARKERTILER_H
