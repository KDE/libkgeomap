/** ===========================================================
 * @file
 *
 * This file is a part of KDE project
 *
 *
 * @date   2011-01-12
 * @brief  Merges tiles into groups
 *
 * @author Copyright (C) 2011 by Michael G. Hansen
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

#ifndef KGEOMAP_TILEGROUPER_H
#define KGEOMAP_TILEGROUPER_H

// local includes

#include "kgeomap_common.h"

namespace KGeoMap
{

class MapBackend;

class TileGrouper : public QObject
{
    Q_OBJECT

public:

    TileGrouper(const QExplicitlySharedDataPointer<KGeoMapSharedData>& sharedData, QObject* const parent);
    ~TileGrouper();

    void setClustersDirty();
    bool getClustersDirty() const;
    void updateClusters();
    void setCurrentBackend(MapBackend* const backend);

private:

    bool currentBackendReady();

private:

    class Private;
    const QScopedPointer<Private> d;

    const QExplicitlySharedDataPointer<KGeoMapSharedData> s;
};

} // namespace KGeoMap

#endif // KGEOMAP_TILEGROUPER_H
