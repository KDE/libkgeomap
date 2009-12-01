/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : Google-Maps-backend for WorldMapWidget2
 *
 * Copyright (C) 2009 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef BACKEND_GOOGLEMAPS_H
#define BACKEND_GOOGLEMAPS_H

// local includes

#include "map-backend.h"

namespace WMW2 {

class BackendGoogleMapsPrivate;

class BackendGoogleMaps : public MapBackend
{
Q_OBJECT

public:
    BackendGoogleMaps(QObject* const parent = 0);
    virtual ~BackendGoogleMaps();

    virtual QString backendName() const;
    virtual QWidget* mapWidget() const;

private:
    BackendGoogleMapsPrivate* const d;
};

} /* WMW2 */

#endif /* BACKEND_GOOGLEMAPS_H */

