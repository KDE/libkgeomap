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

// Local includes

#include "groupstatecomputer.h"
#include "types.h"
#include "libkgeomap_debug.h"
// #include "geocoordinates.h"

namespace KGeoMap
{

class Q_DECL_HIDDEN GroupStateComputer::Private
{
public:

    Private()
      : state(SelectedNone),
        stateMask(SelectedNone)
    {
    }

    GroupState state;
    GroupState stateMask;
};

GroupStateComputer::GroupStateComputer()
    : d(new Private)
{
}

GroupStateComputer::~GroupStateComputer()
{
}

GroupState GroupStateComputer::getState() const
{
    return d->state;
}

void GroupStateComputer::clear()
{
    d->state     = SelectedNone;
    d->stateMask = SelectedNone;
}

void GroupStateComputer::addState(const GroupState state)
{
    addSelectedState(state);
    addFilteredPositiveState(state);
    addRegionSelectedState(state);
}

void GroupStateComputer::addSelectedState(const GroupState state)
{
    if (!(d->stateMask & SelectedMask))
    {
        d->state     |= state;
        d->stateMask |= SelectedMask;
    }
    else
    {
        if ((state&SelectedMask) == SelectedAll)
        {
            d->state |= SelectedAll;
        }
        else if ((d->state&SelectedMask) == SelectedAll)
        {
            d->state |= SelectedSome;
        }
        else
        {
            d->state |= state;
        }
    }
}

void GroupStateComputer::addFilteredPositiveState(const GroupState state)
{
    if (!(d->stateMask & FilteredPositiveMask))
    {
        d->state     |= state;
        d->stateMask |= FilteredPositiveMask;
    }
    else
    {
        if ((state&FilteredPositiveMask) == FilteredPositiveAll)
        {
            d->state |= FilteredPositiveAll;
        }
        else if ((d->state&FilteredPositiveMask) == FilteredPositiveAll)
        {
            d->state |= FilteredPositiveSome;
        }
        else
        {
            d->state |= state;
        }
    }
}

void GroupStateComputer::addRegionSelectedState(const GroupState state)
{
    if (!(d->stateMask & RegionSelectedMask))
    {
        d->state     |= state;
        d->stateMask |= RegionSelectedMask;
    }
    else
    {
        if ((state&RegionSelectedMask) == RegionSelectedAll)
        {
            d->state |= RegionSelectedAll;
        }
        else if ((d->state&RegionSelectedMask) == RegionSelectedAll)
        {
            d->state |= RegionSelectedSome;
        }
        else
        {
            d->state |= state;
        }
    }
}

} /* namespace KGeoMap */
