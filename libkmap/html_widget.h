/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  Widget for displaying HTML in the backends
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
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

#ifndef HTML_WIDGET_H
#define HTML_WIDGET_H

// KDE includes

#include <khtml_part.h>

// local includes

#include "kmap_primitives.h"
#include "libkmap_export.h"

namespace KMapIface
{

class KMAP_EXPORT HTMLWidget : public KHTMLPart
{
    Q_OBJECT

public:

    HTMLWidget(QWidget* const parent = 0);
    ~HTMLWidget();

    void loadInitialHTML(const QString& initialHTML);
    QVariant runScript(const QString& scriptCode);
    bool runScript2Coordinates(const QString& scriptCode, WMWGeoCoordinate* const coordinates);
    void mouseModeChanged(bool state);
    void setSelectionRectangle(const QList<qreal>& searchCoordinates);
    QList<qreal> getSelectionRectangle();

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
    void selectionHasBeenMade(const QList<qreal>& coordinatesRect);

private:

    class HTMLWidgetPrivate;
    HTMLWidgetPrivate* const d;
};

} /* namespace KMapIface */

#endif /* HTML_WIDGET_H */
