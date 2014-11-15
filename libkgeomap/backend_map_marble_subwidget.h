/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-08
 * @brief  Internal part of the Marble-backend for KGeoMap
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

#ifndef BACKEND_MAP_MARBLE_SUBWIDGET_H
#define BACKEND_MAP_MARBLE_SUBWIDGET_H

// Qt includes

#include <QtCore/QPointer>

// KDE includes

#include <marble/MarbleWidget.h>

namespace KGeoMap
{

class BackendMarble;

class BMWidget : public Marble::MarbleWidget
{
    Q_OBJECT

public:

    explicit BMWidget(BackendMarble* const pMarbleBackend, QWidget* const parent = 0);
    virtual ~BMWidget();

protected:

    virtual void customPaint(Marble::GeoPainter* painter);

private:

    QPointer<BackendMarble> const marbleBackend;
};

} /* namespace KGeoMap */

#endif /* BACKEND_MAP_MARBLE_SUBWIDGET_H */
