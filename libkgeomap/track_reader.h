/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2006-09-19
 * @brief  Track file reader
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


#ifndef TRACK_READER_H
#define TRACK_READER_H

#include "track_reader.h"

// Qt includes

#include <QXmlDefaultHandler>

// local includes

#include "tracks.h"

class TestTracks;

namespace KGeoMap
{

class KGEOMAP_EXPORT TrackReader : public QXmlDefaultHandler
{
public:

    explicit TrackReader(TrackManager::Track* const dataTarget);

    virtual bool characters(const QString& ch);
    virtual bool endElement(const QString& namespaceURI, const QString& localName, const QString& qName);
    virtual bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts);

    static TrackManager::Track loadTrackFile(const KUrl& url);
    static QDateTime ParseTime(QString timeString);

private:

    void rebuildElementPath();

    static QString myQName(const QString& namespaceURI, const QString& localName);

private:

    TrackManager::Track* const        fileData;
    QString                           currentElementPath;
    QStringList                       currentElements;
    QString                           currentText;
    TrackManager::TrackPoint          currentDataPoint;
    bool                              verifyFoundGPXElement;

    friend class ::TestTracks;
};

} /* KGeoMap */

#endif /* TRACK_READER_H */
