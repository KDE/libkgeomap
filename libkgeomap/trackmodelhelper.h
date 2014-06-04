/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2014-06-04
 * @brief  Helper class for access to track models.
 *
 * @author Copyright (C) 2009-2011, 2014 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2014 by Justus Schwartz
 *         <a href="mailto:justus at gmx dot li">justus at gmx dot li</a>
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

#ifndef KGEOMAP_TRACKMODELHELPER
#define KGEOMAP_TRACKMODELHELPER

// Qt includes

#include <QtCore/QPoint>

// Kde includes

#include <kdebug.h>

// local includes

#include "libkgeomap_export.h"
#include "kgeomap_primitives.h"
#include "geocoordinates.h"

namespace KGeoMap
{

class KGEOMAP_EXPORT TrackModelHelper : public QObject
{
    Q_OBJECT

public:
    explicit TrackModelHelper(QObject* const parent = 0);
    virtual ~TrackModelHelper();

    virtual QList<GeoCoordinates::List> getTracks() const = 0;
    
Q_SIGNALS:

    void signalModelChanged();
};

}


#endif /* KGEOMAP_TRACKMODELHELPER */
