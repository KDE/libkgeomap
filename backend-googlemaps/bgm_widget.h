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

#ifndef BGM_WIDGET_H
#define BGM_WIDGET_H

// KDE includes

#include <khtml_part.h>

// local includes

#include "worldmapwidget2_primitives.h"

namespace WMW2 {

class BGMWidgetPrivate;

class BGMWidget : public KHTMLPart
{
Q_OBJECT

public:
    BGMWidget(QWidget* const parent = 0);
    ~BGMWidget();

    void loadInitialHTML(const WMWGeoCoordinate& initialCenter = WMWGeoCoordinate(52.0, 6.0), const QString& initialMapType = "ROADMAP");

protected:
    bool eventFilter(QObject* object, QEvent* event);
    void khtmlMousePressEvent(khtml::MousePressEvent* e);
    void khtmlMouseReleaseEvent(khtml::MouseReleaseEvent* e);
    void khtmlMouseMoveEvent(khtml::MouseMoveEvent *e);
    void scanForJSMessages();

protected Q_SLOTS:
    void slotHTMLCompleted();

Q_SIGNALS:
    void signalHTMLEvents(const QStringList& events);

private:
    BGMWidgetPrivate* const d;
};


} /* WMW2 */

#endif /* BGM_WIDGET_H */

