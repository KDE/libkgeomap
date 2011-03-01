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

#include "backend_altitude_geonames.moc"

// KDE includes

#include <kio/job.h>
#include <klocale.h>

namespace KMap
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

    KMapAltitudeLookup::List lookups;
    QByteArray              data;
    KIO::Job*               kioJob;
};

// --------------------------------------------------------------------------

class BackendAltitudeGeonames::BackendAltitudeGeonamesPrivate
{
public:

    BackendAltitudeGeonamesPrivate()
        : jobs()
    {
    }

    QList<MergedAltitudeQueryJobs> jobs;
};

// --------------------------------------------------------------------------

BackendAltitudeGeonames::BackendAltitudeGeonames(const QExplicitlySharedDataPointer<KMapSharedData>& sharedData,
                                                 QObject* const parent)
                       : AltitudeBackend(sharedData, parent), d(new BackendAltitudeGeonamesPrivate)
{
}

BackendAltitudeGeonames::~BackendAltitudeGeonames()
{
    delete d;
}

QString BackendAltitudeGeonames::backendName() const
{
    return QLatin1String("geonames");
}

QString BackendAltitudeGeonames::backendHumanName() const
{
    return i18n("geonames.org");
}

bool BackendAltitudeGeonames::queryAltitudes(const KMapAltitudeLookup::List& queryItems)
{
    // merge queries with the same coordinates into one query:
    // TODO: use a QMap instead to speed it up
    QList<MergedAltitudeQueryJobs> mergedJobs;
    for (int i = 0; i<queryItems.count(); ++i)
    {
        KMapAltitudeLookup query = queryItems.at(i);
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

            GeoCoordinates queryCoordinates = myQuery.lookups.first().coordinates;

            if (!latString.isEmpty())
            {
                latString+=QLatin1Char( ',' );
                lonString+=QLatin1Char( ',' );
            }

            latString+=queryCoordinates.latString();
            lonString+=queryCoordinates.lonString();
        }

        KUrl jobUrl("http://ws.geonames.org/srtm3");
        jobUrl.addQueryItem(QLatin1String("lats" ), latString);
        jobUrl.addQueryItem(QLatin1String("lngs" ), lonString);

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
    KMAP_ASSERT(kioJob!=0);

    for (int i=0; i<d->jobs.count(); ++i)
    {
        if (d->jobs.at(i).kioJob == kioJob)
        {
            MergedAltitudeQueryJobs myJob = d->jobs.takeAt(i);

            QStringList altitudes = QString(QLatin1String( myJob.data )).split(QRegExp( QLatin1String("\\s+" ) ) );

            int jobIndex = 0;
            foreach(const QString& altitudeString, altitudes)
            {
                bool haveAltitude = false;
                qreal altitude = altitudeString.toFloat(&haveAltitude);
                kDebug()<<altitude;

                // -32786 means that geonames.org has no data for these coordinates
                if (altitude==-32768)
                {
                    haveAltitude = false;
                }

                const GeoCoordinates firstJobCoordinates = myJob.lookups.at(jobIndex).coordinates;
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

} /* namespace KMap */
