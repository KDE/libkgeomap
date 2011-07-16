/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2011-04-30
 * @brief  Class for geonames.org based altitude lookup
 *
 * @author Copyright (C) 2010, 2011 by Michael G. Hansen
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

#ifndef LOOKUP_ALTITUDE_GEONAMES_H
#define LOOKUP_ALTITUDE_GEONAMES_H

// local includes

#include "lookup_altitude.h"

/// @cond false
namespace KIO
{
    class Job;
}
class KJob;
/// @endcond

namespace KMap
{

class KGEOMAP_EXPORT LookupAltitudeGeonames : public LookupAltitude
{
    Q_OBJECT

public:

    LookupAltitudeGeonames(QObject* const parent);
    virtual ~LookupAltitudeGeonames();

    virtual QString backendName() const;
    virtual QString backendHumanName() const;

    virtual void addRequests(const Request::List& requests);
    virtual Request::List getRequests() const;
    virtual Request getRequest(const int index) const;

    virtual void startLookup();
    virtual Status getStatus() const;
    virtual QString errorMessage() const;
    virtual void cancel();

private Q_SLOTS:

    void slotData(KIO::Job* kioJob, const QByteArray& data);
    void slotResult(KJob* kJob);

private:

    void startNextRequest();

    class LookupAltitudeGeonamesPrivate;
    LookupAltitudeGeonamesPrivate* const d;
};

} /* namespace KMap */

#endif /* LOOKUP_ALTITUDE_GEONAMES_H */
