/** ===========================================================
 * @file
 *
 * This file is a part of KDE project
 *
 *
 * @date   2009-12-01
 * @brief  class GroupState
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

#ifndef KGEOMAP_GROUPSTATE_H
#define KGEOMAP_GROUPSTATE_H

#include <QtCore/QFlags>

namespace KGeoMap
{

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
 * Selected___: An object is selected.
 * FilteredPositive___: An object was highlighted by a filter. This usually
 *                   means that not-positively-filtered objects should be hidden.
 * RegionSelected___: An object is inside a region of interest on the map.
 */
enum GroupStateEnum
{
    SelectedMask         = 0x03 << 0,
    SelectedNone         = 0x00 << 0,
    SelectedSome         = 0x03 << 0,
    SelectedAll          = 0x02 << 0,

    FilteredPositiveMask = 0x03 << 2,
    FilteredPositiveNone = 0x00 << 2,
    FilteredPositiveSome = 0x03 << 2,
    FilteredPositiveAll  = 0x02 << 2,

    RegionSelectedMask   = 0x03 << 4,
    RegionSelectedNone   = 0x00 << 4,
    RegionSelectedSome   = 0x03 << 4,
    RegionSelectedAll    = 0x02 << 4
};

/// @todo GroupState -> GroupStates?
Q_DECLARE_FLAGS(GroupState, GroupStateEnum)
Q_DECLARE_OPERATORS_FOR_FLAGS(GroupState)

} // namespace KGeoMap

#endif // KGEOMAP_GROUPSTATE_H
