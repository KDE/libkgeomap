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

#include "html_widget.moc"

// Qt includes

#include <QTimer>
#include <QPainter>

// KDE includes

#include <khtmlview.h>
#include <khtml_events.h>

// local includes

#include "kmap_common.h"
#include "kmap_primitives.h"

namespace KMapIface
{

class HTMLWidget::HTMLWidgetPrivate
{
public:

    HTMLWidgetPrivate()
      : parent(0),
        isReady(false),
        javascriptScanTimer(0),
        selectionStatus(false),
        firstSelectionPoint(),
        secondSelectionPoint()
    {
    }

    QWidget*            parent;
    bool                isReady;
    QTimer*             javascriptScanTimer;

    bool                selectionStatus;
    WMWGeoCoordinate    firstSelectionPoint;
    WMWGeoCoordinate    intermediateSelectionPoint;
    WMWGeoCoordinate    secondSelectionPoint;
    QList<qreal>        selectionRectangle;
};

HTMLWidget::HTMLWidget(QWidget* const parent)
          : KHTMLPart(parent), d(new HTMLWidgetPrivate())
{
    d->parent = parent;

    widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // create a timer for monitoring for javascript events, but do not start it yet:
    d->javascriptScanTimer = new QTimer(this);
    d->javascriptScanTimer->setSingleShot(false);
    d->javascriptScanTimer->setInterval(300);
    connect(d->javascriptScanTimer, SIGNAL(timeout()),
            this, SLOT(slotScanForJSMessages()));

    connect(this, SIGNAL(completed()),
            this, SLOT(slotHTMLCompleted()));

    if (d->parent)
    {
        d->parent->installEventFilter(this);
    }
}

HTMLWidget::~HTMLWidget()
{
    delete d;
}

void HTMLWidget::loadInitialHTML(const QString& initialHTML)
{
//     kDebug()<<initialHTML;
    begin();
    write(initialHTML);
    end();
}

void HTMLWidget::slotHTMLCompleted()
{
    d->isReady = true;

    // start monitoring for javascript events using a timer:
    d->javascriptScanTimer->start();

    emit(signalJavaScriptReady());
}

void HTMLWidget::khtmlMousePressEvent(khtml::MousePressEvent* e)
{

    slotScanForJSMessages();
    KHTMLPart::khtmlMousePressEvent(e);
}

void HTMLWidget::khtmlMouseReleaseEvent(khtml::MouseReleaseEvent* e)
{

    if(d->selectionStatus)
    {
        if(!d->firstSelectionPoint.hasCoordinates())
        {
            runScript2Coordinates( QString("wmwPixelToLatLng(%1, %2);")
                                    .arg(e->x())
                                    .arg(e->y()),
                                   &d->firstSelectionPoint);

            runScript(QString("addSelectionPoint(%1, %2, 'red');").arg(d->firstSelectionPoint.lon()).arg(d->firstSelectionPoint.lat()));//.arg("red"));
        }
        else
        {

            if(!d->secondSelectionPoint.hasCoordinates())
            {

                runScript2Coordinates( QString("wmwPixelToLatLng(%1, %2);")
                                        .arg(e->x())
                                        .arg(e->y()),
                                        &d->secondSelectionPoint);

                qreal lonWest  = d->firstSelectionPoint.lon();
                qreal latNorth = d->firstSelectionPoint.lat();
                qreal lonEast  = d->secondSelectionPoint.lon();
                qreal latSouth = d->secondSelectionPoint.lat();

                if(lonWest > lonEast)
                {
                    const qreal auxCoord = lonWest;
                    lonWest              = lonEast;
                    lonEast              = auxCoord;
                }

                if(latNorth < latSouth)
                {
                    const qreal auxCoord = latNorth;
                    latNorth             = latSouth;
                    latSouth             = auxCoord;
                }

                runScript(QString("addSelectionPoint(%1, %2, 'blue');").arg(d->secondSelectionPoint.lon()).arg(d->secondSelectionPoint.lat())); //.arg("blue"));

                QList<qreal> selectionCoordinates;
                selectionCoordinates << lonWest << latNorth << lonEast << latSouth;

                emit selectionHasBeenMade(selectionCoordinates); 
            }
        }
    }

    slotScanForJSMessages();
    KHTMLPart::khtmlMouseReleaseEvent(e);
}

void HTMLWidget::khtmlMouseMoveEvent(khtml::MouseMoveEvent *e)
{

    if(d->selectionStatus && d->firstSelectionPoint.hasCoordinates() && !d->secondSelectionPoint.hasCoordinates())
    {
        runScript2Coordinates( QString("wmwPixelToLatLng(%1, %2);")
                                    .arg(e->x())
                                    .arg(e->y()),
                                   &d->intermediateSelectionPoint);

        runScript(QString("addSelectionPoint(%1, %2, 'red');").arg(d->intermediateSelectionPoint.lon()).arg(d->intermediateSelectionPoint.lat()));  //.arg("red"));
    }

    slotScanForJSMessages();
    KHTMLPart::khtmlMouseMoveEvent(e);
}

void HTMLWidget::slotScanForJSMessages()
{
    const QString status = jsStatusBarText();

    if (status!="(event)")
        return;

    kDebug()<<status;
    const QString eventBufferString = runScript(QString("wmwReadEventStrings();")).toString();
    if (eventBufferString.isEmpty())
        return;

    const QStringList events = eventBufferString.split('|');

    emit(signalHTMLEvents(events));
}

/**
 * @brief Wrapper around executeScript to catch more errors
 */
QVariant HTMLWidget::runScript(const QString& scriptCode)
{
    KMAP_ASSERT(d->isReady);

    if (!d->isReady)
        return QVariant();

//     kDebug()<<scriptCode;
    return executeScript(scriptCode);
}

/**
 * @brief Execute a script which returns coordinates and parse these
 */
bool HTMLWidget::runScript2Coordinates(const QString& scriptCode, WMWGeoCoordinate* const coordinates)
{
    const QVariant scriptResult = runScript(scriptCode);

    return WMWHelperParseLatLonString(scriptResult.toString(), coordinates);
}

bool HTMLWidget::eventFilter(QObject* object, QEvent* event)
{
    if (d->parent && object==d->parent)
    {

        if (event->type()==QEvent::Resize)
        {
            QResizeEvent* const resizeEvent = dynamic_cast<QResizeEvent*>(event);
            if (resizeEvent)
            {
                widget()->resize(resizeEvent->size());
                view()->resize(resizeEvent->size());
            }
        }
    }
    return false;
}

void HTMLWidget::setSearchRectangle(const QList<qreal>& searchCoordinates)
{
    
}

void HTMLWidget::mouseModeChanged(bool state)
{
    d->selectionStatus = state;
    if(d->selectionStatus == false)
    {
        d->firstSelectionPoint.clear();
        d->secondSelectionPoint.clear();
    }
    runScript(QString("selectionModeStatus(%1);").arg(state)); 
}

} /* namespace KMapIface */
