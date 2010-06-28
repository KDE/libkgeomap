/* ============================================================
 *
 * Date        : 2010-02-13
 * Description : Base class for altitude lookup backends
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "altitude-backend.moc"

namespace WMW2
{

AltitudeBackend::AltitudeBackend(const QExplicitlySharedDataPointer<WMWSharedData>& sharedData, QObject* const parent)
: QObject(parent), s(sharedData)
{

}

AltitudeBackend::~AltitudeBackend()
{
}

} /* WMW2 */

