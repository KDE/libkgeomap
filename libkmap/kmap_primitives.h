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
 * either version 2, or (at your option) any later version.
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

#include <QtCore/QPersistentModelIndex>
#include <QtCore/QString>
#include <QtCore/QVariant>

// Kde includes

#include <kdebug.h>
#include <kurl.h>

// local includes

#include "libkmap_export.h"
#include "geocoordinates.h"

Q_DECLARE_METATYPE(QPersistentModelIndex)

#ifdef KMAP_HAVE_VALGRIND
#include <valgrind/valgrind.h>
#endif /* KMAP_HAVE_VALGRIND */

#define KMAP_ASSERT(cond) ((!(cond)) ? KMap::KMap_assert(#cond,__FILE__,__LINE__) : qt_noop())

namespace KMap
{

inline void KMap_assert(const char* const condition, const char* const filename, const int lineNumber)
{
    const QString debugString = QString::fromLatin1( "ASSERT: %1 - %2:%3").arg(QLatin1String( condition )).arg(QLatin1String( filename )).arg(lineNumber);
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
    MouseModePan                        = 1,
    MouseModeRegionSelection            = 2,
    MouseModeRegionSelectionFromIcon    = 4,
    MouseModeFilter                     = 8,
    MouseModeSelectThumbnail            = 16,
    MouseModeZoomIntoGroup              = 32,
    MouseModeLast                       = 32
};

Q_DECLARE_FLAGS(MouseModes, MouseMode)
Q_DECLARE_OPERATORS_FOR_FLAGS(MouseModes)

enum ExtraAction
{
    ExtraActionSticky = 1
};

Q_DECLARE_FLAGS(ExtraActions, ExtraAction)
Q_DECLARE_OPERATORS_FOR_FLAGS(ExtraActions)

typedef QList<int>      QIntList;
typedef QPair<int, int> QIntPair;

/**
 * @brief Representation of possible tile or cluster states
 *
 * The idea is that a group consists of more than one object.
 * Thus the resulting state is that either none of the objects,
 * some or all of them have a certain state. The constants for each
 * state are set up such that they can be logically or'ed: If a group
 * has the state ___All, and another the state ___Some, the bit
 * representing ___Some is always propagated along. You only have to
 * make sure that once you reach an object with ___None, and the computed
 * state is ___All, to set the ___Some bit.
 *
 * KMapSelected___: An object is selected.
 * KMapFilteredPositive___: An object was highlighted by a filter. This usually
 *                          means that not-positively-filtered objects should be hidden.
 * KMapRegionSelected___: An object is inside a region of interest on the map.
 */
enum KMapGroupStateEnum
{
    KMapSelectedMask = 0x03 << 0,
    KMapSelectedNone = 0x00 << 0,
    KMapSelectedSome = 0x03 << 0,
    KMapSelectedAll  = 0x02 << 0,

    KMapFilteredPositiveMask = 0x03 << 2,
    KMapFilteredPositiveNone = 0x00 << 2,
    KMapFilteredPositiveSome = 0x03 << 2,
    KMapFilteredPositiveAll  = 0x02 << 2,

    KMapRegionSelectedMask = 0x03 << 4,
    KMapRegionSelectedNone = 0x00 << 4,
    KMapRegionSelectedSome = 0x03 << 4,
    KMapRegionSelectedAll  = 0x02 << 4
};

/// @todo KMapGroupState -> KMapGroupStates?
Q_DECLARE_FLAGS(KMapGroupState, KMapGroupStateEnum)
Q_DECLARE_OPERATORS_FOR_FLAGS(KMapGroupState)

class KMapGroupStateComputer
{
private:

    KMapGroupState m_state;
    KMapGroupState m_stateMask;

public:

    /// @todo Make member functions non-inline?

    KMapGroupStateComputer()
    : m_state(KMapSelectedNone),
      m_stateMask(KMapSelectedNone)
    {
    }

    KMapGroupState getState() const
    {
        return m_state;
    }

    void clear()
    {
        m_state = KMapSelectedNone;
        m_stateMask = KMapSelectedNone;
    }

    void addState(const KMapGroupState state)
    {
        addSelectedState(state);
        addFilteredPositiveState(state);
        addRegionSelectedState(state);
    }

    void addSelectedState(const KMapGroupState state)
    {
        if (!(m_stateMask & KMapSelectedMask))
        {
            m_state|= state;
            m_stateMask|= KMapSelectedMask;
        }
        else
        {
            if ((state&KMapSelectedMask)==KMapSelectedAll)
            {
                m_state|=KMapSelectedAll;
            }
            else if ((m_state&KMapSelectedMask)==KMapSelectedAll)
            {
                m_state|=KMapSelectedSome;
            }
            else
            {
                m_state|=state;
            }
        }
    }

    void addFilteredPositiveState(const KMapGroupState state)
    {
        if (!(m_stateMask & KMapFilteredPositiveMask))
        {
            m_state|= state;
            m_stateMask|= KMapFilteredPositiveMask;
        }
        else
        {
            if ((state&KMapFilteredPositiveMask)==KMapFilteredPositiveAll)
            {
                m_state|=KMapFilteredPositiveAll;
            }
            else if ((m_state&KMapFilteredPositiveMask)==KMapFilteredPositiveAll)
            {
                m_state|=KMapFilteredPositiveSome;
            }
            else
            {
                m_state|=state;
            }
        }
    }

    void addRegionSelectedState(const KMapGroupState state)
    {
        if (!(m_stateMask & KMapRegionSelectedMask))
        {
            m_state|= state;
            m_stateMask|= KMapRegionSelectedMask;
        }
        else
        {
            if ((state&KMapRegionSelectedMask)==KMapRegionSelectedAll)
            {
                m_state|=KMapRegionSelectedAll;
            }
            else if ((m_state&KMapRegionSelectedMask)==KMapRegionSelectedAll)
            {
                m_state|=KMapRegionSelectedSome;
            }
            else
            {
                m_state|=state;
            }
        }
    }
};

// primitives for altitude lookup:
class KMAP_EXPORT KMapAltitudeLookup
{
public:

    GeoCoordinates                   coordinates;
    QVariant                         data;

    typedef QList<KMapAltitudeLookup> List;
};

} /* namespace KMap */

Q_DECLARE_METATYPE(KMap::KMapAltitudeLookup)
Q_DECLARE_METATYPE(KMap::MouseModes)

#endif /* KMAP_PRIMITIVES_H */
