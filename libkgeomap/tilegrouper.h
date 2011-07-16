/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
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

#ifndef TILEGROUPER_H
#define TILEGROUPER_H

// local includes

#include "kmap_common.h"

namespace KMap
{

class MapBackend;

class TileGrouper : public QObject
{
    Q_OBJECT

public:

    TileGrouper(const QExplicitlySharedDataPointer<KMapSharedData>& sharedData, QObject* const parent);
    ~TileGrouper();

    void setClustersDirty();
    bool getClustersDirty() const;
    void updateClusters();
    void setCurrentBackend(MapBackend* const backend);

private:

    bool currentBackendReady();

    class TileGrouperPrivate;
    TileGrouperPrivate* const d;

    const QExplicitlySharedDataPointer<KMapSharedData> s;

};

} /* KMap */

#endif /* TILEGROUPER_H */
