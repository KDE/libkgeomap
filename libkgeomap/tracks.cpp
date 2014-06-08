/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2006-09-19
 * @brief  Track file loading and managing
 *
 * @author Copyright (C) 2006-2013 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2010, 2014 by Michael G. Hansen
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

#include "tracks.moc"

// Qt includes

#include <qtconcurrentmap.h>
#include <QFuture>
#include <QFutureWatcher>
#include <QString>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// local includes

#include "track_reader.h"

namespace KGeoMap
{

// TrackManager::TrackPoint ---------------------------------------------------

bool TrackManager::TrackPoint::EarlierThan(const TrackPoint& a, const TrackPoint& b)
{
    return a.dateTime < b.dateTime;
}

// TrackManager ---------------------------------------------------------------

class TrackManager::Private
{
public:

    Private()
      : trackLoadFutureWatcher(0),
        trackLoadFuture(),
        trackList(),
        loadErrorFiles()
    {

    }

    QFutureWatcher<TrackManager::Track>* trackLoadFutureWatcher;
    QFuture<TrackManager::Track>         trackLoadFuture;
    TrackManager::Track::List            trackList;
    QList<QPair<KUrl, QString> >         loadErrorFiles;
};

TrackManager::TrackManager(QObject* const parent)
    : QObject(parent), d(new Private())
{

}

TrackManager::~TrackManager()
{

}

void TrackManager::clear()
{
    d->trackList.clear();
}

const TrackManager::Track& TrackManager::getTrack(const int index) const
{
    return d->trackList.at(index);
}

void TrackManager::loadTrackFiles(const KUrl::List& urls)
{
    d->trackLoadFutureWatcher = new QFutureWatcher<Track>(this);

    connect(d->trackLoadFutureWatcher, SIGNAL(resultsReadyAt(int,int)),
            this, SLOT(slotTrackFilesReadyAt(int,int)));

    connect(d->trackLoadFutureWatcher, SIGNAL(finished()),
            this, SLOT(slotTrackFilesFinished()));

    d->trackLoadFuture = QtConcurrent::mapped(urls, TrackReader::loadTrackFile);
    d->trackLoadFutureWatcher->setFuture(d->trackLoadFuture);

    // results are reported to slotTrackFilesReadyAt
}

void TrackManager::slotTrackFilesReadyAt(int beginIndex, int endIndex)
{
    const int nFilesBefore = d->trackList.count();

    // note that endIndex is exclusive!
    for (int i=beginIndex; i<endIndex; ++i)
    {
        const Track nextFile = d->trackLoadFuture.resultAt(i);

        if (nextFile.isValid)
        {
            d->trackList << nextFile;
        }
        else
        {
            d->loadErrorFiles << QPair<KUrl, QString>(nextFile.url, nextFile.loadError);
        }
    }

    // note that endIndex is exclusive!
    emit(signalTrackFilesReadyAt(nFilesBefore, d->trackList.count()));
}

void TrackManager::slotTrackFilesFinished()
{
    d->trackLoadFutureWatcher->deleteLater();

    emit(signalAllTrackFilesReady());
}

TrackManager::Track::List TrackManager::getTrackList() const
{
    return d->trackList;
}

int TrackManager::trackCount() const
{
    return d->trackList.count();
}

QList<QPair<KUrl, QString> > TrackManager::readLoadErrors()
{
    const QList<QPair<KUrl, QString> > result = d->loadErrorFiles;
    d->loadErrorFiles.clear();

    return result;
}

} // namespace KGeoMap
