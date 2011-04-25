/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2011-04-25
 * @brief  Helper class to get backends for lookups
 *
 * @author Copyright (C) 2011 by Michael G. Hansen
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

#ifndef BACKEND_HELPER_H
#define BACKEND_HELPER_H

// Local includes

#include "kmap_common.h"

namespace KMap
{

class AltitudeBackend;

class KMAP_EXPORT BackendHelper
{
public:

    static QStringList getAltitudeBackendNames();
    static AltitudeBackend* getAltitudeBackend(const QString& backendName, QObject* const parent);
};

} /* namespace KMap */

#endif /* BACKEND_HELPER_H */
