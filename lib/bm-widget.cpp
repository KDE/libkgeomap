/* ============================================================
 *
 * Date        : 2009-12-08
 * Description : Marble-backend for WorldMapWidget2
 *
 * Copyright (C) 2009 by Michael G. Hansen <mike at mghansen dot de>
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

#include "bm-widget.moc"

// local includes

#include "backend-marble.h"

namespace WMW2 {

BMWidget::BMWidget(BackendMarble* const pMarbleBackend, QWidget* const parent)
: Marble::MarbleWidget(parent), marbleBackend(pMarbleBackend)
{
    WMW2_ASSERT(marbleBackend!=0);
}

BMWidget::~BMWidget()
{
}

void BMWidget::customPaint(Marble::GeoPainter* painter)
{
    if (marbleBackend)
    {
        marbleBackend->marbleCustomPaint(painter);
    }
}

} /* WMW2 */



