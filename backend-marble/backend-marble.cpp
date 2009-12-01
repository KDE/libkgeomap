/* ============================================================
 *
 * Date        : 2009-12-01
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

// Qt includes

#include <QPointer>

// KDE includes

#include <marble/MarbleWidget.h>

// local includes

#include "backend-marble.h"

using namespace Marble;

namespace WMW2 {

class BackendMarblePrivate
{
public:
    BackendMarblePrivate()
    : marbleWidget(0)
    {
    }

    QPointer<MarbleWidget> marbleWidget;
};

BackendMarble::BackendMarble(QObject* const parent)
: MapBackend(parent), d(new BackendMarblePrivate())
{
    d->marbleWidget = new MarbleWidget();
    d->marbleWidget->setMapThemeId(QLatin1String("earth/srtm/srtm.dgml"));

    d->marbleWidget->setShowCompass(false);
    d->marbleWidget->setShowScaleBar(false);
    d->marbleWidget->setShowOverviewMap(false);

}

BackendMarble::~BackendMarble()
{
    if (d->marbleWidget)
        delete d->marbleWidget;
    
    delete d;
}

QString BackendMarble::backendName() const
{
    return "marble";
}

QWidget* BackendMarble::mapWidget() const
{
    return d->marbleWidget;
}

} /* WMW2 */

