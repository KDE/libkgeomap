/** ===========================================================
 * @file
 *
 * This file is a part of KDE project
 *
 *
 * @date   2011-04-30
 * @brief  Base class for altitude lookup jobs
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

#ifndef KGEOMAP_LOOKUP_ALTITUDE_H
#define KGEOMAP_LOOKUP_ALTITUDE_H

#include <QtCore/QObject>

// local includes

#include "libkgeomap_export.h"
#include "geocoordinates.h"

namespace KGeoMap
{

class KGEOMAP_EXPORT LookupAltitude : public QObject
{
    Q_OBJECT

public:

    class KGEOMAP_EXPORT Request
    {
    public:

        Request()
            : coordinates(),
              success(false),
              data()
        {
        }

        GeoCoordinates         coordinates;
        bool                   success;
        QVariant               data;

        typedef QList<Request> List;
    };

public:

    enum StatusEnum
    {
        StatusInProgress = 0,
        StatusSuccess    = 1,
        StatusCanceled   = 2,
        StatusError      = 3
    };
    Q_DECLARE_FLAGS(Status, StatusEnum)

public:

    explicit LookupAltitude(QObject* const parent);
    ~LookupAltitude() override;

    virtual QString backendName() const = 0;
    virtual QString backendHumanName() const = 0;

    virtual void addRequests(const Request::List& requests) = 0;
    virtual Request::List getRequests() const = 0;
    virtual Request getRequest(const int index) const = 0;

    virtual void startLookup() = 0;
    virtual Status getStatus() const = 0;
    virtual QString errorMessage() const = 0;
    virtual void cancel() = 0;

Q_SIGNALS:

    void signalRequestsReady(const QList<int>& readyRequests);
    void signalDone();
};

} // namespace KGeoMap

Q_DECLARE_OPERATORS_FOR_FLAGS(KGeoMap::LookupAltitude::Status)

#endif // KGEOMAP_LOOKUP_ALTITUDE_H
