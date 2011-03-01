/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-02-13
 * @brief  geonames.org based altitude lookup backend
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gilles Caulier
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

#ifndef BACKEND_ALTITUDE_GEONAMES_H
#define BACKEND_ALTITUDE_GEONAMES_H

// Local includes

#include "backend_altitude.h"

/// @cond false
namespace KIO
{
    class Job;
}
/// @endcond

class KJob;

namespace KMap
{

class BackendAltitudeGeonames : public AltitudeBackend
{
    Q_OBJECT

public:

    BackendAltitudeGeonames(const QExplicitlySharedDataPointer<KMapSharedData>& sharedData, QObject* const parent);
    virtual ~BackendAltitudeGeonames();

    virtual QString backendName() const;
    virtual QString backendHumanName() const;

    virtual bool queryAltitudes(const KMapAltitudeLookup::List& queryItems);

private Q_SLOTS:

    void slotData(KIO::Job* kioJob, const QByteArray& data);
    void slotResult(KJob* kJob);

private:

    class BackendAltitudeGeonamesPrivate;
    BackendAltitudeGeonamesPrivate* const d;
};

} /* namespace KMap */

#endif /* BACKEND_ALTITUDE_GEONAMES_H */
