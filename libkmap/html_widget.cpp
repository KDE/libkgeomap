/** ===========================================================
 * @file
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
 * either version 2, or (at your option) any later version.
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

namespace KMap
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
        secondSelectionPoint(),
        currentMouseMode(MouseModePan),
        firstSelectionScreenPoint(),
        secondSelectionScreenPoint()
    {
    }

    QWidget*            parent;
    bool                isReady;
    QTimer*             javascriptScanTimer;

    bool                selectionStatus;
    GeoCoordinates    firstSelectionPoint;
    GeoCoordinates    intermediateSelectionPoint;
    GeoCoordinates    secondSelectionPoint;
    GeoCoordinates::Pair displayedRectangle;
    MouseModes          currentMouseMode;
    QPoint              firstSelectionScreenPoint;
    QPoint              secondSelectionScreenPoint;
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

    if(d->currentMouseMode == MouseModeSelection)
    {
        if(!d->firstSelectionPoint.hasCoordinates())
        {
            runScript2Coordinates( QString("wmwPixelToLatLng(%1, %2);")
                                    .arg(e->x())
                                    .arg(e->y()),
                                   &d->firstSelectionPoint);

            d->firstSelectionScreenPoint = QPoint(e->x(), e->y());

        }
        else
        {

            if(!d->secondSelectionPoint.hasCoordinates())
            {

                runScript2Coordinates( QString("wmwPixelToLatLng(%1, %2);")
                                        .arg(e->x())
                                        .arg(e->y()),
                                        &d->secondSelectionPoint);

                d->secondSelectionScreenPoint = QPoint(e->x(), e->y());

                qreal lonWest, latNorth, lonEast, latSouth;
                if(d->firstSelectionScreenPoint.x() < d->secondSelectionScreenPoint.x())
                {
                    lonWest  = d->firstSelectionPoint.lon();
                    lonEast  = d->secondSelectionPoint.lon();
                }
                else
                {
                    lonEast  = d->firstSelectionPoint.lon();
                    lonWest  = d->secondSelectionPoint.lon(); 
                }

                if(d->firstSelectionScreenPoint.y() < d->secondSelectionScreenPoint.y())
                {
                    latNorth = d->firstSelectionPoint.lat();
                    latSouth = d->secondSelectionPoint.lat();
                }
                else
                {
                    latNorth = d->secondSelectionPoint.lat();
                    latSouth = d->firstSelectionPoint.lat();
                }

                runScript(QString("setDisplayedRectangle(%1, %2, %3, %4);").arg(lonWest).arg(latNorth).arg(lonEast).arg(latSouth));
                runScript(QString("removeSelectionRectangle();"));              
 
                const GeoCoordinates::Pair selectionCoordinates(
                        GeoCoordinates(latNorth, lonWest),
                        GeoCoordinates(latSouth, lonEast)
                    );

                emit selectionHasBeenMade(selectionCoordinates);

                d->firstSelectionPoint.clear();
                d->intermediateSelectionPoint.clear();
                d->secondSelectionPoint.clear();
                runScript(QString("clearSelectionPoints();"));

                d->displayedRectangle = selectionCoordinates; 
            }
        }
    }

    slotScanForJSMessages();
    KHTMLPart::khtmlMouseReleaseEvent(e);
}

void HTMLWidget::khtmlMouseMoveEvent(khtml::MouseMoveEvent *e)
{

    if(d->currentMouseMode == MouseModeSelection && d->firstSelectionPoint.hasCoordinates() && !d->secondSelectionPoint.hasCoordinates())
    {
        runScript2Coordinates( QString("wmwPixelToLatLng(%1, %2);")
                                    .arg(e->x())
                                    .arg(e->y()),
                                   &d->intermediateSelectionPoint);

        d->secondSelectionScreenPoint = QPoint(e->x(), e->y());

        kDebug()<<d->firstSelectionScreenPoint<<" "<<d->secondSelectionScreenPoint;

        qreal lonWest, latNorth, lonEast, latSouth;
        if(d->firstSelectionScreenPoint.x() < d->secondSelectionScreenPoint.x())
        {
            lonWest  = d->firstSelectionPoint.lon();
            lonEast  = d->intermediateSelectionPoint.lon();
        }
        else
        {
            lonEast  = d->firstSelectionPoint.lon();
            lonWest  = d->intermediateSelectionPoint.lon(); 
        }

        if(d->firstSelectionScreenPoint.y() < d->secondSelectionScreenPoint.y())
        {
            latNorth = d->firstSelectionPoint.lat();
            latSouth = d->intermediateSelectionPoint.lat();
        }
        else
        {
            latNorth = d->intermediateSelectionPoint.lat();
            latSouth = d->firstSelectionPoint.lat();
        }

        runScript(QString("setSelectionRectangle(%1, %2, %3, %4, 'red');").arg(lonWest).arg(latNorth).arg(lonEast).arg(latSouth));
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
bool HTMLWidget::runScript2Coordinates(const QString& scriptCode, GeoCoordinates* const coordinates)
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

void HTMLWidget::setSelectionRectangle(const GeoCoordinates::Pair& searchCoordinates)
{
    if(!searchCoordinates.first.hasCoordinates())
    {
        d->displayedRectangle.first.clear();
        return;
    }

    qreal West  = searchCoordinates.first.lon();
    qreal North = searchCoordinates.first.lat();
    qreal East  = searchCoordinates.second.lon();
    qreal South = searchCoordinates.second.lat();
    
    runScript(QString("setDisplayedRectangle(%1, %2, %3, %4);").arg(West).arg(North).arg(East).arg(South));
    runScript(QString("clearSelectionPoints();"));

    d->displayedRectangle = searchCoordinates;
}

GeoCoordinates::Pair HTMLWidget::getSelectionRectangle()
{
    return d->displayedRectangle;
}

void HTMLWidget::removeSelectionRectangle()
{
    if(!d->displayedRectangle.first.hasCoordinates())
        return;
    d->displayedRectangle.first.clear();
    runScript(QString("removeDisplayedRectangle();"));
}

void HTMLWidget::mouseModeChanged(const MouseModes mouseMode)
{
    bool state;
    d->currentMouseMode = mouseMode;

    if(d->currentMouseMode != MouseModeSelection)
    {
        d->firstSelectionPoint.clear();
        d->secondSelectionPoint.clear();
        state = false;
        runScript(QString("selectionModeStatus(%1);").arg(state)); 
    }
    else
    {
        state = true;
        runScript(QString("selectionModeStatus(%1);").arg(state)); 
    }
}

void HTMLWidget::centerOn(const qreal west, const qreal north, const qreal east, const qreal south, const bool useSaneZoomLevel)
{
//    kDebug()<<"West:"<<west<<" North:"<<north<<" East:"<<east<<" South:"<<south;
    runScript(QString("setMapBoundaries(%1, %2, %3, %4, %5);").arg(west).arg(north).arg(east).arg(south).arg(useSaneZoomLevel?1:0));
}

} /* namespace KMap */
