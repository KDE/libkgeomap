/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-08
 * @brief  Marble-backend for WorldMapWidget2
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
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

#include "backend-marble-subwidget.moc"

// local includes

#include "backend-marble.h"

namespace KMapIface
{

BMWidget::BMWidget(BackendMarble* const pMarbleBackend, QWidget* const parent)
        : Marble::MarbleWidget(parent), marbleBackend(pMarbleBackend)
{
    KMAP_ASSERT(marbleBackend!=0);
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

} /* namespace KMapIface */


