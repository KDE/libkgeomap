/** ===========================================================
 * @file
 *
 * This file is a part of KDE project
 *
 *
 * @date   2011-04-30
 * @brief  Class for geonames.org based altitude lookup
 *
 * @author Copyright (C) 2010-2011 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010-2015 by Gilles Caulier
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

#ifndef KGEOMAP_LOOKUP_ALTITUDE_GEONAMES_H
#define KGEOMAP_LOOKUP_ALTITUDE_GEONAMES_H

// local includes

#include "lookupaltitude.h"

/// @cond false
namespace KIO
{
    class Job;
}

class KJob;
/// @endcond

namespace KGeoMap
{

class LookupAltitudeGeonames : public LookupAltitude
{
    Q_OBJECT

public:

    explicit LookupAltitudeGeonames(QObject* const parent);
    ~LookupAltitudeGeonames() override;

    QString backendName() const override;
    QString backendHumanName() const override;

    void addRequests(const Request::List& requests) override;
    Request::List getRequests() const override;
    Request getRequest(const int index) const override;

    void startLookup() override;
    Status getStatus() const override;
    QString errorMessage() const override;
    void cancel() override;

private Q_SLOTS:

    void slotData(KIO::Job* kioJob, const QByteArray& data);
    void slotResult(KJob* kJob);

private:

    void startNextRequest();

private:

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace KGeoMap

#endif // KGEOMAP_LOOKUP_ALTITUDE_GEONAMES_H
