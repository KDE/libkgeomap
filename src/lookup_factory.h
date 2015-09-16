/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
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

#ifndef LOOKUP_FACTORY_H
#define LOOKUP_FACTORY_H

// local includes

#include "kgeomap_primitives.h"
#include "libkgeomap_export.h"

namespace KGeoMap
{

class LookupAltitude;

class KGEOMAP_EXPORT LookupFactory
{
public:

    static LookupAltitude* getAltitudeLookup(const QString& backendName, QObject* const parent);
};

} /* namespace KGeoMap */

#endif /* LOOKUP_FACTORY_H */
