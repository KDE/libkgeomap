/** ===========================================================
 * @file
 *
 * This file is a part of KDE project
 *
 *
 * @date   2011-05-05
 * @brief  Factory to create instances of Lookup backends
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

#include "lookupfactory.h"

// local includes

#include "lookupaltitudegeonames.h"

namespace KGeoMap
{

LookupAltitude* LookupFactory::getAltitudeLookup(const QString& backendName, QObject* const parent)
{
    if (backendName == QLatin1String("geonames"))
    {
        return new LookupAltitudeGeonames(parent);
    }

    return nullptr;
}

} /* namespace KGeoMap */

