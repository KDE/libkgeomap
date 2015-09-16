#ifndef GROUPSTATE_H
#define GROUPSTATE_H

#include <QFlags>

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

} /* namespace KGeoMap */

#endif // GROUPSTATE_H
