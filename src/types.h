/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  Primitive datatypes for KGeoMap
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
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

#ifndef KGEOMAP_TYPES_H
#define KGEOMAP_TYPES_H

// Qt includes

#include <QtCore/QPersistentModelIndex>

Q_DECLARE_METATYPE(QPersistentModelIndex)

namespace KGeoMap
{

enum MouseMode
{
    MouseModePan                     = 1,
    MouseModeRegionSelection         = 2,
    MouseModeRegionSelectionFromIcon = 4,
    MouseModeFilter                  = 8,
    MouseModeSelectThumbnail         = 16,
    MouseModeZoomIntoGroup           = 32,
    MouseModeLast                    = 32
};

Q_DECLARE_FLAGS(MouseModes, MouseMode)
Q_DECLARE_OPERATORS_FOR_FLAGS(MouseModes)

enum ExtraAction
{
    ExtraActionSticky = 1
};

Q_DECLARE_FLAGS(ExtraActions, ExtraAction)
Q_DECLARE_OPERATORS_FOR_FLAGS(ExtraActions)

typedef QList<int> QIntList;

} // namespace KGeoMap

Q_DECLARE_METATYPE(KGeoMap::MouseModes)

#endif // KGEOMAP_TYPES_H
