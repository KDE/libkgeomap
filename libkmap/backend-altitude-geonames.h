/* ============================================================
 *
 * Date        : 2010-02-13
 * Description : geonames.org based altitude lookup backend
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef BACKEND_ALTITUDE_GEONAMES_H
#define BACKEND_ALTITUDE_GEONAMES_H

#include "altitude-backend.h"

namespace KIO { class Job; }
class KJob;

namespace WMW2
{

class BackendAltitudeGeonamesPrivate;

class BackendAltitudeGeonames : public AltitudeBackend
{
Q_OBJECT

public:
    BackendAltitudeGeonames(const QExplicitlySharedDataPointer<WMWSharedData>& sharedData, QObject* const parent);
    virtual ~BackendAltitudeGeonames();

    virtual QString backendName() const;
    virtual QString backendHumanName() const;

    virtual bool queryAltitudes(const WMWAltitudeLookup::List& queryItems);

private Q_SLOTS:
    void slotData(KIO::Job* kioJob, const QByteArray& data);
    void slotResult(KJob* kJob);

private:
    BackendAltitudeGeonamesPrivate* const d;
};

} /* WMW2 */

#endif /* BACKEND_ALTITUDE_GEONAMES_H */
