/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2006-09-19
 * @brief  Track file reader
 *
 * @author Copyright (C) 2006-2014 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2010-2014 by Michael G. Hansen
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

#include "track_reader.h"

// Qt includes

#include <QDomDocument>
#include <QFile>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

namespace KGeoMap
{

class TrackReader::Private
{
public:

    Private()
        : fileData(0),
          verifyFoundGPXElement(false)
    {
    }

    TrackReadResult*         fileData;
    QString                  currentElementPath;
    QStringList              currentElements;
    QString                  currentText;
    TrackManager::TrackPoint currentDataPoint;
    bool                     verifyFoundGPXElement;
};    
    
TrackReader::TrackReader(TrackReadResult* const dataTarget)
    : QXmlDefaultHandler(),
      d(new Private)
{
    d->fileData = dataTarget;
}

TrackReader::~TrackReader()
{
    delete d;
}

QDateTime TrackReader::ParseTime(QString timeString)
{
    if (timeString.isEmpty())
    {
        return QDateTime();
    }

    // we want to be able to parse these formats:
    // "2010-01-14T09:26:02.287-02:00" <-- here we have to cut off the -02:00 and replace it with 'Z'
    // "2010-01-14T09:26:02.287+02:00" <-- here we have to cut off the +02:00 and replace it with 'Z'
    // "2009-03-11T13:39:55.622Z"

    const int timeStringLength      = timeString.length();
    const int timeZoneSignPosition  = timeStringLength-6;

    // does the string contain a timezone offset?
    int timeZoneOffsetSeconds       = 0;
    const int timeZonePlusPosition  = timeString.lastIndexOf("+");
    const int timeZoneMinusPosition = timeString.lastIndexOf("-");

    if ( (timeZonePlusPosition == timeZoneSignPosition) || (timeZoneMinusPosition == timeZoneSignPosition) )
    {
        const int timeZoneSign       = (timeZonePlusPosition == timeZoneSignPosition) ? +1 : -1;

        // cut off the last digits:
        const QString timeZoneString = timeString.right(6);
        timeString.chop(6);
        timeString                  += 'Z';

        // determine the time zone offset:
        bool okayHour                = false;
        bool okayMinute              = false;
        const int hourOffset         = timeZoneString.mid(1, 2).toInt(&okayHour);
        const int minuteOffset       = timeZoneString.mid(4, 2).toInt(&okayMinute);

        if (okayHour&&okayMinute)
        {
            timeZoneOffsetSeconds  = hourOffset*3600 + minuteOffset*60;
            timeZoneOffsetSeconds *= timeZoneSign;
        }
    }

    QDateTime theTime = QDateTime::fromString(timeString, Qt::ISODate);
    theTime           = theTime.addSecs(-timeZoneOffsetSeconds);

    return theTime;
}

/**
 * @brief The parser found characters
 */
bool TrackReader::characters(const QString& ch)
{
    d->currentText += ch;
    return true;
}

static QString GPX10("http://www.topografix.com/GPX/1/0");
static QString GPX11("http://www.topografix.com/GPX/1/1");

QString TrackReader::myQName(const QString& namespaceURI, const QString& localName)
{
    if ( (namespaceURI == GPX10)  ||
         (namespaceURI == GPX11) )
    {
        return "gpx:"+localName;
    }

    return namespaceURI+localName;
}

bool TrackReader::endElement(const QString& namespaceURI, const QString& localName, const QString& qName)
{
    Q_UNUSED(qName)

    // we always work with the old path
    const QString ePath = d->currentElementPath;
    const QString eText = d->currentText.trimmed();
    const QString eName = myQName(namespaceURI, localName);
    d->currentElements.removeLast();
    d->currentText.clear();
    rebuildElementPath();

    if (ePath == "gpx:gpx/gpx:trk/gpx:trkseg/gpx:trkpt")
    {
        if (d->currentDataPoint.dateTime.isValid() && d->currentDataPoint.coordinates.hasCoordinates())
        {
            d->fileData->track.points << d->currentDataPoint;
        }

        d->currentDataPoint = TrackManager::TrackPoint();
    }
    else if (ePath == "gpx:gpx/gpx:trk/gpx:trkseg/gpx:trkpt/gpx:time")
    {
        d->currentDataPoint.dateTime = ParseTime(eText.trimmed());
    }
    else if (ePath == "gpx:gpx/gpx:trk/gpx:trkseg/gpx:trkpt/gpx:sat")
    {
        bool okay       = false;
        int nSatellites = eText.toInt(&okay);

        if (okay && (nSatellites >= 0))
            d->currentDataPoint.nSatellites = nSatellites;
    }
    else if (ePath == "gpx:gpx/gpx:trk/gpx:trkseg/gpx:trkpt/gpx:hdop")
    {
        bool okay  = false;
        qreal hDop = eText.toDouble(&okay);

        if (okay)
            d->currentDataPoint.hDop = hDop;
    }
    else if (ePath == "gpx:gpx/gpx:trk/gpx:trkseg/gpx:trkpt/gpx:pdop")
    {
        bool okay  = false;
        qreal pDop = eText.toDouble(&okay);

        if (okay)
            d->currentDataPoint.pDop = pDop;
    }
    else if (ePath == "gpx:gpx/gpx:trk/gpx:trkseg/gpx:trkpt/gpx:fix")
    {
        int fixType = -1;

        if (eText=="2d")
        {
            fixType = 2;
        }
        else if (eText=="3d")
        {
            fixType = 3;
        }

        if (fixType>=0)
        {
            d->currentDataPoint.fixType = fixType;
        }
    }
    else if (ePath == "gpx:gpx/gpx:trk/gpx:trkseg/gpx:trkpt/gpx:ele")
    {
        bool haveAltitude = false;
        const qreal alt   = eText.toDouble(&haveAltitude);

        if (haveAltitude)
        {
            d->currentDataPoint.coordinates.setAlt(alt);
        }
    }
    else if (ePath == "gpx:gpx/gpx:trk/gpx:trkseg/gpx:trkpt/gpx:speed")
    {
        bool haveSpeed    = false;
        const qreal speed = eText.toDouble(&haveSpeed);

        if (haveSpeed)
        {
            d->currentDataPoint.speed = speed;
        }
    }

    return true;
}

bool TrackReader::startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts)
{
    Q_UNUSED(qName)

    const QString eName  = myQName(namespaceURI, localName);
    d->currentElements << eName;
    rebuildElementPath();
    const QString& ePath = d->currentElementPath;

    if (ePath == "gpx:gpx/gpx:trk/gpx:trkseg/gpx:trkpt")
    {
        qreal lat    = 0.0;
        qreal lon    = 0.0;
        bool haveLat = false;
        bool haveLon = false;

        for (int i = 0; i < atts.count(); ++i)
        {
            const QString attName  = myQName(atts.uri(i), atts.localName(i));
            const QString attValue = atts.value(i);

            if (attName == "lat")
            {
                lat = attValue.toDouble(&haveLat);
            }
            else if (attName == "lon")
            {
                lon = attValue.toDouble(&haveLon);
            }
        }

        if (haveLat&&haveLon)
        {
            d->currentDataPoint.coordinates.setLatLon(lat, lon);
        }
    }
    else if (ePath=="gpx:gpx")
    {
        d->verifyFoundGPXElement = true;
    }

    return true;
}

void TrackReader::rebuildElementPath()
{
    d->currentElementPath = d->currentElements.join("/");
}

TrackReader::TrackReadResult TrackReader::loadTrackFile(const KUrl& url)
{
    // TODO: store some kind of error message
    TrackReadResult parsedData;
    parsedData.track.url = url;
    parsedData.isValid   = false;

    QFile file(url.toLocalFile());

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        parsedData.loadError = i18n("Could not open: %1", file.errorString());
        return parsedData;
    }

    if (file.size() == 0)
    {
        parsedData.loadError = i18n("File is empty.");
        return parsedData;
    }

    // TODO: load the file
    TrackReader trackReader(&parsedData);
    QXmlSimpleReader reader;
    reader.setContentHandler(&trackReader);
    reader.setErrorHandler(&trackReader);
    QXmlInputSource xmlInputSource(&file);

    // TODO: error handling
    parsedData.isValid = reader.parse(xmlInputSource);

    if (!parsedData.isValid)
    {
        parsedData.loadError = i18n("Parsing error: %1", trackReader.errorString());
        return parsedData;
    }

    parsedData.isValid = !parsedData.track.points.isEmpty();

    if (!parsedData.isValid)
    {
        if (!trackReader.d->verifyFoundGPXElement)
        {
            parsedData.loadError = i18n("No GPX element found - probably not a GPX file.");
        }
        else
        {
            parsedData.loadError = i18n("File is a GPX file, but no datapoints were found.");
        }

        return parsedData;
    }

    // the correlation algorithm relies on sorted data, therefore sort now
    qSort(parsedData.track.points.begin(), parsedData.track.points.end(), TrackManager::TrackPoint::EarlierThan);

    return parsedData;
}

} // namespace KGeoMap
