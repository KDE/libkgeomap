/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-02-13
 * @brief  Base class for altitude lookup backends
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

#ifndef BACKEND_ALTITUDE_H
#define BACKEND_ALTITUDE_H

// Local includes

#include "kmap_common.h"

namespace KMap
{

class KMAP_EXPORT AltitudeBackend : public QObject
{
    Q_OBJECT

public:

    AltitudeBackend(const QExplicitlySharedDataPointer<KMapSharedData>& sharedData, QObject* const parent);
    virtual ~AltitudeBackend();

    virtual QString backendName() const = 0;
    virtual QString backendHumanName() const = 0;

    virtual bool queryAltitudes(const KMapAltitudeLookup::List& queryItems) = 0;

Q_SIGNALS:

    void signalAltitudes(const KMap::KMapAltitudeLookup::List results);

public:

    const QExplicitlySharedDataPointer<KMapSharedData> s;
};

} /* namespace KMap */

#endif /* BACKEND_ALTITUDE_H */
