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

#include "kgeomap_primitives.h"

// Local includes

#include "geocoordinates.h"
#include "libkgeomap_debug.h"

namespace KGeoMap
{

class Q_DECL_HIDDEN KGeoMapGroupStateComputer::Private
{
public:

    Private()
      : state(KGeoMapSelectedNone),
        stateMask(KGeoMapSelectedNone)
    {
    }

    KGeoMapGroupState state;
    KGeoMapGroupState stateMask;
};

KGeoMapGroupStateComputer::KGeoMapGroupStateComputer()
    : d(new Private)
{
}

KGeoMapGroupStateComputer::~KGeoMapGroupStateComputer()
{
}

KGeoMapGroupState KGeoMapGroupStateComputer::getState() const
{
    return d->state;
}

void KGeoMapGroupStateComputer::clear()
{
    d->state     = KGeoMapSelectedNone;
    d->stateMask = KGeoMapSelectedNone;
}

void KGeoMapGroupStateComputer::addState(const KGeoMapGroupState state)
{
    addSelectedState(state);
    addFilteredPositiveState(state);
    addRegionSelectedState(state);
}

void KGeoMapGroupStateComputer::addSelectedState(const KGeoMapGroupState state)
{
    if (!(d->stateMask & KGeoMapSelectedMask))
    {
        d->state     |= state;
        d->stateMask |= KGeoMapSelectedMask;
    }
    else
    {
        if ((state&KGeoMapSelectedMask) == KGeoMapSelectedAll)
        {
            d->state |= KGeoMapSelectedAll;
        }
        else if ((d->state&KGeoMapSelectedMask) == KGeoMapSelectedAll)
        {
            d->state |= KGeoMapSelectedSome;
        }
        else
        {
            d->state |= state;
        }
    }
}

void KGeoMapGroupStateComputer::addFilteredPositiveState(const KGeoMapGroupState state)
{
    if (!(d->stateMask & KGeoMapFilteredPositiveMask))
    {
        d->state     |= state;
        d->stateMask |= KGeoMapFilteredPositiveMask;
    }
    else
    {
        if ((state&KGeoMapFilteredPositiveMask) == KGeoMapFilteredPositiveAll)
        {
            d->state |= KGeoMapFilteredPositiveAll;
        }
        else if ((d->state&KGeoMapFilteredPositiveMask) == KGeoMapFilteredPositiveAll)
        {
            d->state |= KGeoMapFilteredPositiveSome;
        }
        else
        {
            d->state |= state;
        }
    }
}

void KGeoMapGroupStateComputer::addRegionSelectedState(const KGeoMapGroupState state)
{
    if (!(d->stateMask & KGeoMapRegionSelectedMask))
    {
        d->state     |= state;
        d->stateMask |= KGeoMapRegionSelectedMask;
    }
    else
    {
        if ((state&KGeoMapRegionSelectedMask) == KGeoMapRegionSelectedAll)
        {
            d->state |= KGeoMapRegionSelectedAll;
        }
        else if ((d->state&KGeoMapRegionSelectedMask) == KGeoMapRegionSelectedAll)
        {
            d->state |= KGeoMapRegionSelectedSome;
        }
        else
        {
            d->state |= state;
        }
    }
}

} /* namespace KGeoMap */
