/** ===========================================================
 * @file
 *
 * This file is a part of KDE project
 *
 *
 * @date   2009-12-01
 * @brief  An abstract base class for tiling of markers
 *
 * @author Copyright (C) 2009-2011 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
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

#ifndef KGEOMAP_ABSTRACTMARKERTILER_H
#define KGEOMAP_ABSTRACTMARKERTILER_H

// Qt includes

#include <QtCore/QBitArray>
#include <QtCore/QObject>
#include <QtCore/QPoint>

// local includes

#include "tileindex.h"
#include "types.h"
#include "libkgeomap_export.h"
#include "groupstate.h"

namespace KGeoMap
{

class KGEOMAP_EXPORT AbstractMarkerTiler : public QObject
{
    Q_OBJECT

public:

    enum Flag
    {
        FlagNull    = 0,
        FlagMovable = 1
    };

    Q_DECLARE_FLAGS(Flags, Flag)

public:

    class ClickInfo
    {
    public:

        TileIndex::List   tileIndicesList;
        QVariant          representativeIndex;
        GroupState        groupSelectionState;
        MouseModes        currentMouseMode;
    };

public:

    class Tile
    {
    public:

        Tile();

        /**
         * NOTE: Tile is only deleted by AbstractMarkerTiler::tileDelete.
         * All subclasses of AbstractMarkerTiler have to reimplement tileDelete
         * to delete their Tile subclasses.
         * This was done in order not to have any virtual functions
         * in Tile and its subclasses in order to save memory, since there
         * can be a lot of tiles in a MarkerTiler.
         */
        ~Tile();

        Tile* getChild(const int linearIndex);

        void addChild(const int linearIndex, Tile* const tilePointer);

        /**
         * @brief Sets the pointer to a child tile to zero, but you have to delete the tile by yourself!
         */
        void clearChild(const int linearIndex);

        int indexOfChildTile(Tile* const tile);

        bool childrenEmpty() const;

        /**
         * @brief Take away the list of children, only to be used for deleting them.
         *
         * @todo Make this function protected.
         *
         */
        QVector<Tile*> takeChildren();

        static int maxChildCount();

    private:

        void prepareForChildren();

    private:

        QVector<Tile*> children;

    };

public:

    class NonEmptyIterator
    {
    public:

        NonEmptyIterator(AbstractMarkerTiler* const model, const int level);
        NonEmptyIterator(AbstractMarkerTiler* const model, const int level, const TileIndex& startIndex, const TileIndex& endIndex);
        NonEmptyIterator(AbstractMarkerTiler* const model, const int level, const GeoCoordinates::PairList& normalizedMapBounds);
        ~NonEmptyIterator();

        bool                 atEnd()        const;
        TileIndex            nextIndex();
        TileIndex            currentIndex() const;
        AbstractMarkerTiler* model()        const;

    private:

        bool initializeNextBounds();

    private:

        class Private;
        Private* const d;
    };

public:

    explicit AbstractMarkerTiler(QObject* const parent = nullptr);
    ~AbstractMarkerTiler() override;

    void tileDeleteChildren(Tile* const tile);
    void tileDelete(Tile* const tile);
    void tileDeleteChild(Tile* const parentTile, Tile* const childTile, const int knownLinearIndex = -1);

    // these have to be implemented
    virtual Flags tilerFlags() const;
    virtual Tile* tileNew();
    virtual void tileDeleteInternal(Tile* const tile);
    virtual void prepareTiles(const GeoCoordinates& upperLeft, const GeoCoordinates& lowerRight, int level) = 0;
    virtual void regenerateTiles() = 0;
    virtual Tile* getTile(const TileIndex& tileIndex, const bool stopIfEmpty = false) = 0;
    virtual int getTileMarkerCount(const TileIndex& tileIndex) = 0;
    virtual int getTileSelectedCount(const TileIndex& tileIndex) = 0;

    // these should be implemented for thumbnail handling
    virtual QVariant getTileRepresentativeMarker(const TileIndex& tileIndex, const int sortKey) = 0;
    virtual QVariant bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey) = 0;
    virtual QPixmap pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size) = 0;
    virtual bool indicesEqual(const QVariant& a, const QVariant& b) const = 0;
    virtual GroupState getTileGroupState(const TileIndex& tileIndex) = 0;
    virtual GroupState getGlobalGroupState() = 0;

    // these can be implemented if you want to react to actions in kgeomap
    virtual void onIndicesClicked(const ClickInfo& clickInfo);
    virtual void onIndicesMoved(const TileIndex::List& tileIndicesList, const GeoCoordinates& targetCoordinates,
                                const QPersistentModelIndex& targetSnapIndex);

    virtual void setActive(const bool state) = 0;
    Tile* rootTile();
    bool indicesEqual(const QIntList& a, const QIntList& b, const int upToLevel) const;
    bool isDirty() const;
    void setDirty(const bool state = true);
    Tile* resetRootTile();

Q_SIGNALS:

    void signalTilesOrSelectionChanged();
    void signalThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap);

protected:

    /**
     * @brief Only used to safely delete all tiles in the desctructor
     */
    void clear();

private:

    class Private;
    Private* const d;
};

} // namespace KGeoMap

Q_DECLARE_OPERATORS_FOR_FLAGS(KGeoMap::AbstractMarkerTiler::Flags)

#endif // KGEOMAP_ABSTRACTMARKERTILER_H
