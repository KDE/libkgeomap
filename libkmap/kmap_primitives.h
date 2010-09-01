/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  Primitive datatypes for KMap
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef KMAP_PRIMITIVES_H
#define KMAP_PRIMITIVES_H

// Qt includes

#include <QPersistentModelIndex>
#include <QString>
#include <QStringList>
#include <QVariant>

// Kde includes

#include <kdebug.h>
#include <kurl.h>

// local includes

#include "libkmap_export.h"
#include "kmap_geocoordinates.h"

Q_DECLARE_METATYPE(QPersistentModelIndex)

#ifdef KMAP_HAVE_VALGRIND
#include <valgrind/valgrind.h>
#endif /* KMAP_HAVE_VALGRIND */

#define KMAP_ASSERT(cond) ((!(cond)) ? KMap::KMap_assert(#cond,__FILE__,__LINE__) : qt_noop())

namespace KMap
{

inline void KMap_assert(const char* const condition, const char* const filename, const int lineNumber)
{
    const QString debugString = QString("ASSERT: %1 - %2:%3").arg(condition).arg(filename).arg(lineNumber);
#ifdef KMAP_HAVE_VALGRIND
    if (RUNNING_ON_VALGRIND>0)
    {
        // TODO: which encoding?
        const QByteArray dummyArray = debugString.toUtf8();
        VALGRIND_PRINTF_BACKTRACE("%s", dummyArray.constData());
    }
    else
    {
        kDebug(51006)<<debugString;
    }
#else
    kDebug(51006)<<debugString;
#endif /* KMAP_HAVE_VALGRIND */
}

enum MouseMode
{
    MouseModePan = 1,
    MouseModeSelection = 2,
    MouseModeZoom = 4,
    MouseModeFilter = 8,
    MouseModeSelectThumbnail = 16,
    MouseModeLast = 16
};

Q_DECLARE_FLAGS(MouseModes, MouseMode)
Q_DECLARE_OPERATORS_FOR_FLAGS(MouseModes);

enum ExtraAction
{
    ExtraActionSticky = 1
};

Q_DECLARE_FLAGS(ExtraActions, ExtraAction)
Q_DECLARE_OPERATORS_FOR_FLAGS(ExtraActions);

enum DisplayedRectangles
{
    SelectionRectangle = 0,
    DisplayedRectangle,
    Both
};

typedef QList<int> QIntList;
typedef QPair<int, int> QIntPair;

enum WMWSelectionState
{
    WMWSelectedNone = 0,
    WMWSelectedSome = 1,
    WMWSelectedAll  = 2
};

// primitives for altitude lookup:
class KMAP_EXPORT WMWAltitudeLookup
{
public:

    GeoCoordinates                 coordinates;
    QVariant                         data;

    typedef QList<WMWAltitudeLookup> List;
};

} /* namespace KMap */

Q_DECLARE_METATYPE(KMap::WMWAltitudeLookup)

#endif /* KMAP_PRIMITIVES_H */
