/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  Base-class for backends for KMap
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

#include "map-backend.moc"

namespace KMapIface
{

class MapBackendPrivate
{
public:

    MapBackendPrivate()
    {
    }
};

MapBackend::MapBackend(const QExplicitlySharedDataPointer<WMWSharedData>& sharedData, QObject* const parent)
          : QObject(parent), s(sharedData), d(new MapBackendPrivate())
{
}

MapBackend::~MapBackend()
{
    delete d;
}

void MapBackend::slotThumbnailAvailableForIndex(const QVariant& /*index*/, const QPixmap& /*pixmap*/)
{
}

} /* namespace KMapIface */
