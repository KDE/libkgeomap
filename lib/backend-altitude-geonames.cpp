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

// KDE includes

#include <kio/job.h>
#include <klocale.h>

// local includes

#include "backend-altitude-geonames.h"

namespace WMW2
{

class MergedAltitudeQueryJobs
{
public:
    MergedAltitudeQueryJobs()
    : lookups(),
      data(),
      kioJob(0)
    {
    }

    WMWAltitudeLookup::List lookups;
    QByteArray data;
    KIO::Job* kioJob;
};

class BackendAltitudeGeonamesPrivate
{
public:
    BackendAltitudeGeonamesPrivate()
    : jobs()
    {
    }

    QList<MergedAltitudeQueryJobs> jobs;
    
};

BackendAltitudeGeonames::BackendAltitudeGeonames(WMWSharedData* const sharedData, QObject* const parent)
: AltitudeBackend(sharedData, parent), d(new BackendAltitudeGeonamesPrivate)
{
}

BackendAltitudeGeonames::~BackendAltitudeGeonames()
{
}

QString BackendAltitudeGeonames::backendName() const
{
    return "geonames";
}

QString BackendAltitudeGeonames::backendHumanName() const
{
    return i18n("geonames.org");
}

bool BackendAltitudeGeonames::queryAltitudes(const WMWAltitudeLookup::List& queryItems)
{
    // merge queries with the same coordinates into one query:
    // TODO: use a QMap instead to speed it up
    QList<MergedAltitudeQueryJobs> mergedJobs;
    for (int i = 0; i<queryItems.count(); ++i)
    {
        WMWAltitudeLookup query = queryItems.at(i);
        query.coordinates.clearAlt();

        bool foundIt = false;
        for (int j = 0; j<mergedJobs.count(); ++j)
        {
            if (mergedJobs.at(j).lookups.first().coordinates.sameLonLatAs(query.coordinates))
            {
                mergedJobs[j].lookups << query;
                foundIt = true;
                break;
            }
        }

        if (!foundIt)
        {
            MergedAltitudeQueryJobs newMergedQuery;
            newMergedQuery.lookups << query;

            mergedJobs << newMergedQuery;
        }
    }

    // geonames.org allows up to 20 lookups in one query. Merge the merged lookups into bunches of 20 and send them off:
    while (!mergedJobs.isEmpty())
    {
        QString latString;
        QString lonString;
        MergedAltitudeQueryJobs newMergedQuery;
        for (int i=0; i<qMin(20,mergedJobs.count()); ++i)
        {
            const MergedAltitudeQueryJobs& myQuery = mergedJobs.takeFirst();
            newMergedQuery.lookups << myQuery.lookups;

            WMWGeoCoordinate queryCoordinates = myQuery.lookups.first().coordinates;

            if (!latString.isEmpty())
            {
                latString+=',';
                lonString+=',';
            }

            latString+=queryCoordinates.latString();
            lonString+=queryCoordinates.lonString();
        }

        KUrl jobUrl("http://ws.geonames.org/srtm3");
        jobUrl.addQueryItem("lats", latString);
        jobUrl.addQueryItem("lngs", lonString);

        // TODO: KIO::NoReload ?
        // TODO: limit the number of concurrent queries
        newMergedQuery.kioJob = KIO::get(jobUrl, KIO::NoReload, KIO::HideProgressInfo);
        d->jobs << newMergedQuery;

        connect(newMergedQuery.kioJob, SIGNAL(data(KIO::Job*, const QByteArray&)),
                this, SLOT(slotData(KIO::Job*, const QByteArray&)));

        connect(newMergedQuery.kioJob, SIGNAL(result(KJob*)),
                this, SLOT(slotResult(KJob*)));
    }
    return false;
}

void BackendAltitudeGeonames::slotData(KIO::Job* kioJob, const QByteArray& data)
{
    for (int i=0; i<d->jobs.count(); ++i)
    {
        if (d->jobs.at(i).kioJob == kioJob)
        {
            d->jobs[i].data.append(data);
            break;
        }
    }
}

void BackendAltitudeGeonames::slotResult(KJob* kJob)
{
    KIO::Job* kioJob = qobject_cast<KIO::Job*>(kJob);
    WMW2_ASSERT(kioJob!=0);

    for (int i=0; i<d->jobs.count(); ++i)
    {
        if (d->jobs.at(i).kioJob == kioJob)
        {
            MergedAltitudeQueryJobs myJob = d->jobs.takeAt(i);
    
            QStringList altitudes = QString(myJob.data).split(QRegExp("\\s+"));

            int jobIndex = 0;
            foreach(const QString altitudeString, altitudes)
            {
                bool haveAltitude = false;
                qreal altitude = altitudeString.toFloat(&haveAltitude);
                kDebug()<<altitude;

                // -32786 means that geonames.org has no data for these coordinates
                if (altitude==-32768)
                {
                    haveAltitude = false;
                }

                const WMWGeoCoordinate firstJobCoordinates = myJob.lookups.at(jobIndex).coordinates;
                kDebug()<<firstJobCoordinates.geoUrl();
                for (; jobIndex<myJob.lookups.count(); ++jobIndex)
                {
                    if (!firstJobCoordinates.sameLonLatAs(myJob.lookups.at(jobIndex).coordinates))
                        break;

                    if (haveAltitude)
                    {
                        myJob.lookups[jobIndex].coordinates.setAlt(altitude);
                    }
                }

                if (jobIndex>=myJob.lookups.count())
                    break;
            }

            emit(signalAltitudes(myJob.lookups));

            // TODO: who gets to delete the KIO::Job?

            break;
        }
    }
}

} /* WMW2 */


