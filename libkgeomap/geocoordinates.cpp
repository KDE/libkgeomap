/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-08-16
 * @brief  GeoCoordinates class
 *
 * @author Copyright (C) 2009-2010, 2014 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010-2014 by Gilles Caulier
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

#include "geocoordinates.h"

// Marble includes

#include <marble/GeoDataCoordinates.h>

namespace KGeoMap
{

Marble::GeoDataCoordinates GeoCoordinates::toMarbleCoordinates() const
{
    Marble::GeoDataCoordinates marbleCoordinates;
    marbleCoordinates.setLongitude(lon(), Marble::GeoDataCoordinates::Degree);
    marbleCoordinates.setLatitude(lat(),  Marble::GeoDataCoordinates::Degree);
    
    if (hasAltitude())
    {
        marbleCoordinates.setAltitude(alt());
    }

    return marbleCoordinates;
}

GeoCoordinates GeoCoordinates::fromMarbleCoordinates(const Marble::GeoDataCoordinates& marbleCoordinates)
{
    /// @TODO looks like Marble does not differentiate between having and not having altitude..
    return GeoCoordinates(
            marbleCoordinates.latitude(Marble::GeoDataCoordinates::Degree),
            marbleCoordinates.longitude(Marble::GeoDataCoordinates::Degree),
            marbleCoordinates.altitude()
        );
}

} /* namespace KGeoMap */