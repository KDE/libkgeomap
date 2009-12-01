/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : Google-Maps-backend for WorldMapWidget2
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

// local includes

#include "bgm_widget.h"


namespace WMW2 {

BGMWidget::BGMWidget(QWidget* const parent)
: KHTMLPart(parent), myParent(parent)
{
    widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // TODO: the khtmlpart-widget does not resize automatically, we have to do it manually???
    parent->installEventFilter(this);
    
    openUrl(KUrl("http://digikam3rdparty.free.fr/gpslocator/getlonlatalt.php?latitude=52&longitude=6&width=400&height=400&altitude=100&zoom=2&filename=bla"));
}

bool BGMWidget::eventFilter(QObject* object, QEvent* event)
{
    if (object==myParent)
    {
        if (event->type()==QEvent::Resize)
        {
            QResizeEvent* const resizeEvent = dynamic_cast<QResizeEvent*>(event);
            if (resizeEvent)
            {
                widget()->resize(resizeEvent->size());
            }
        }
    }
    return false;
}


} /* WMW2 */

