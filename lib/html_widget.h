/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : Widget for displaying HTML in the backends
 *
 * Copyright (C) 2009,2010 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef HTML_WIDGET_H
#define HTML_WIDGET_H

// KDE includes

#include <khtml_part.h>

// local includes

#include "worldmapwidget2_primitives.h"

namespace WMW2 {

class HTMLWidgetPrivate;

class HTMLWidget : public KHTMLPart
{
Q_OBJECT

public:
    HTMLWidget(QWidget* const parent = 0);
    ~HTMLWidget();

    void loadInitialHTML(const QString& initialHTML);
    QVariant runScript(const QString& scriptCode);

protected:
    bool eventFilter(QObject* object, QEvent* event);
    void khtmlMousePressEvent(khtml::MousePressEvent* e);
    void khtmlMouseReleaseEvent(khtml::MouseReleaseEvent* e);
    void khtmlMouseMoveEvent(khtml::MouseMoveEvent *e);

protected Q_SLOTS:
    void slotHTMLCompleted();
    void slotScanForJSMessages();

Q_SIGNALS:
    void signalHTMLEvents(const QStringList& events);
    void signalJavaScriptReady();

private:
    HTMLWidgetPrivate* const d;
};


} /* WMW2 */

#endif /* HTML_WIDGET_H */

