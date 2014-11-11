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
 * @author Copyright (C) 2010-2014 by Gilles Caulier
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

#ifndef KGEOMAP_PRIMITIVES_H
#define KGEOMAP_PRIMITIVES_H

// Qt includes

#include <QtCore/QPersistentModelIndex>
#include <QtCore/QString>
#include <QtCore/QVariant>

// Kde includes

#include <kdebug.h>
#include <kurl.h>

// local includes

#include "libkgeomap_export.h"
#include "geocoordinates.h"

Q_DECLARE_METATYPE(QPersistentModelIndex)

#ifdef KGEOMAP_HAVE_VALGRIND
#include <valgrind/valgrind.h>
#endif /* KGEOMAP_HAVE_VALGRIND */

#define KGEOMAP_ASSERT(cond) ((!(cond)) ? KGeoMap::KGeoMap_assert(#cond,__FILE__,__LINE__) : qt_noop())

namespace KGeoMap
{

inline void KGeoMap_assert(const char* const condition, const char* const filename, const int lineNumber)
{
    const QString debugString = QString::fromLatin1( "ASSERT: %1 - %2:%3").arg(QLatin1String( condition )).arg(QLatin1String( filename )).arg(lineNumber);

#ifdef KGEOMAP_HAVE_VALGRIND
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
#endif /* KGEOMAP_HAVE_VALGRIND */
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
 * KGeoMapSelected___: An object is selected.
 * KGeoMapFilteredPositive___: An object was highlighted by a filter. This usually
 *                          means that not-positively-filtered objects should be hidden.
 * KGeoMapRegionSelected___: An object is inside a region of interest on the map.
 */
enum KGeoMapGroupStateEnum
{
    KGeoMapSelectedMask         = 0x03 << 0,
    KGeoMapSelectedNone         = 0x00 << 0,
    KGeoMapSelectedSome         = 0x03 << 0,
    KGeoMapSelectedAll          = 0x02 << 0,

    KGeoMapFilteredPositiveMask = 0x03 << 2,
    KGeoMapFilteredPositiveNone = 0x00 << 2,
    KGeoMapFilteredPositiveSome = 0x03 << 2,
    KGeoMapFilteredPositiveAll  = 0x02 << 2,

    KGeoMapRegionSelectedMask   = 0x03 << 4,
    KGeoMapRegionSelectedNone   = 0x00 << 4,
    KGeoMapRegionSelectedSome   = 0x03 << 4,
    KGeoMapRegionSelectedAll    = 0x02 << 4
};

/// @todo KGeoMapGroupState -> KGeoMapGroupStates?
Q_DECLARE_FLAGS(KGeoMapGroupState, KGeoMapGroupStateEnum)
Q_DECLARE_OPERATORS_FOR_FLAGS(KGeoMapGroupState)

class KGeoMapGroupStateComputer
{
private:

    KGeoMapGroupState m_state;
    KGeoMapGroupState m_stateMask;

public:

    /// @todo Make member functions non-inline?

    KGeoMapGroupStateComputer()
        : m_state(KGeoMapSelectedNone),
          m_stateMask(KGeoMapSelectedNone)
    {
    }

    KGeoMapGroupState getState() const
    {
        return m_state;
    }

    void clear()
    {
        m_state     = KGeoMapSelectedNone;
        m_stateMask = KGeoMapSelectedNone;
    }

    void addState(const KGeoMapGroupState state)
    {
        addSelectedState(state);
        addFilteredPositiveState(state);
        addRegionSelectedState(state);
    }

    void addSelectedState(const KGeoMapGroupState state)
    {
        if (!(m_stateMask & KGeoMapSelectedMask))
        {
            m_state     |= state;
            m_stateMask |= KGeoMapSelectedMask;
        }
        else
        {
            if ((state&KGeoMapSelectedMask) == KGeoMapSelectedAll)
            {
                m_state |= KGeoMapSelectedAll;
            }
            else if ((m_state&KGeoMapSelectedMask) == KGeoMapSelectedAll)
            {
                m_state |= KGeoMapSelectedSome;
            }
            else
            {
                m_state |= state;
            }
        }
    }

    void addFilteredPositiveState(const KGeoMapGroupState state)
    {
        if (!(m_stateMask & KGeoMapFilteredPositiveMask))
        {
            m_state     |= state;
            m_stateMask |= KGeoMapFilteredPositiveMask;
        }
        else
        {
            if ((state&KGeoMapFilteredPositiveMask) == KGeoMapFilteredPositiveAll)
            {
                m_state |= KGeoMapFilteredPositiveAll;
            }
            else if ((m_state&KGeoMapFilteredPositiveMask) == KGeoMapFilteredPositiveAll)
            {
                m_state |= KGeoMapFilteredPositiveSome;
            }
            else
            {
                m_state |= state;
            }
        }
    }

    void addRegionSelectedState(const KGeoMapGroupState state)
    {
        if (!(m_stateMask & KGeoMapRegionSelectedMask))
        {
            m_state     |= state;
            m_stateMask |= KGeoMapRegionSelectedMask;
        }
        else
        {
            if ((state&KGeoMapRegionSelectedMask) == KGeoMapRegionSelectedAll)
            {
                m_state |= KGeoMapRegionSelectedAll;
            }
            else if ((m_state&KGeoMapRegionSelectedMask) == KGeoMapRegionSelectedAll)
            {
                m_state |= KGeoMapRegionSelectedSome;
            }
            else
            {
                m_state |= state;
            }
        }
    }
};

} /* namespace KGeoMap */

Q_DECLARE_METATYPE(KGeoMap::MouseModes)

#endif /* KGEOMAP_PRIMITIVES_H */
