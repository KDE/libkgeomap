/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-08
 * @brief  Internal part of the Marble-backend for KMap
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

#include "backend_map_marble_subwidget.moc"

// local includes

#include "backend_map_marble.h"

namespace KMap
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

} /* namespace KMap */


