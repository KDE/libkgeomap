/* ============================================================
 *
 * Date        : 2010-02-13
 * Description : Base class for altitude lookup backends
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

#ifndef ALTITUDE_BACKEND_H
#define ALTITUDE_BACKEND_H

#include "kmap_primitives.h"

namespace WMW2
{

class KMAP_EXPORT AltitudeBackend : public QObject
{
Q_OBJECT

public:
    AltitudeBackend(const QExplicitlySharedDataPointer<WMWSharedData>& sharedData, QObject* const parent);
    virtual ~AltitudeBackend();

    virtual QString backendName() const = 0;
    virtual QString backendHumanName() const = 0;

    virtual bool queryAltitudes(const WMWAltitudeLookup::List& queryItems) = 0;

    const QExplicitlySharedDataPointer<WMWSharedData> s;

Q_SIGNALS:
    void signalAltitudes(const WMW2::WMWAltitudeLookup::List results);

};

} /* WMW2 */

#endif /* ALTITUDE_BACKEND_H */

