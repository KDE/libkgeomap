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

// Qt includes

#include <QTimer>

// KDE includes

#include <khtmlview.h>

// local includes

#include "bgm_widget.h"


namespace WMW2 {

class BGMWidgetPrivate
{
public:
    BGMWidgetPrivate()
    : parent(0),
      isReady(false)
    {
    }

    QWidget* parent;
    bool isReady;
};

BGMWidget::BGMWidget(QWidget* const parent)
: KHTMLPart(parent), d(new BGMWidgetPrivate())
{
    d->parent = parent;
    
    widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // TODO: the khtmlpart-widget does not resize automatically, we have to do it manually???
    parent->installEventFilter(this);

    connect(this, SIGNAL(completed()),
            this, SLOT(slotHTMLCompleted()));
}

BGMWidget::~BGMWidget()
{
    delete d;
}

void BGMWidget::loadInitialHTML(const WMWGeoCoordinate& initialCenter, const QString& initialMapType)
{
    QString mapHTMLCode =
"<html>\n"
"<head>\n"
"<script type=\"text/javascript\" src=\"http://maps.google.com/maps/api/js?sensor=false\"></script>\n"
"<script type=\"text/javascript\">\n"
"   var map;\n"
"   var eventBuffer = new Array();\n"
"   var markerList = new Object();\n"
"   function wmwPostEventString(eventString) {\n"
"       eventBuffer.push(eventString);\n"
"       window.status = '(event)';\n"
"   }\n"
"   function wmwReadEventStrings() {\n"
"       var eventBufferString = eventBuffer.join('|');\n"
"       eventBuffer = new Array();\n"
"       return eventBufferString;\n"
"   }\n"
"   function wmwSetZoom(zoomvalue) {\n"
"       map.setZoom(zoomvalue);\n"
"   }\n"
"   function wmwGetZoom() {\n"
"       return map.getZoom();\n"
"   }\n"
"   function wmwZoomIn() {\n"
"       map.setZoom(map.getZoom()+1);\n"
"   }\n"
"   function wmwZoomOut() {\n"
"       map.setZoom(map.getZoom()-1);\n"
"   }\n"
"   function wmwSetCenter(lat, lon) {\n"
"       var latlng = new google.maps.LatLng(lat, lon);\n"
"       map.setCenter(latlng);\n"
"   }\n"
"   function wmwGetCenter() {\n"
"       var latlngString = map.getCenter().toUrlValue(12);\n"
"       return latlngString;\n"
"   }\n"
    // parameter: "SATELLITE"/"ROADMAP"/"HYBRID"/"TERRAIN"
"   function wmwSetMapType(newMapType) {\n"
"       if (newMapType == \"SATELLITE\") { map.setMapTypeId(google.maps.MapTypeId.SATELLITE); }\n"
"       if (newMapType == \"ROADMAP\")   { map.setMapTypeId(google.maps.MapTypeId.ROADMAP); }\n"
"       if (newMapType == \"HYBRID\")    { map.setMapTypeId(google.maps.MapTypeId.HYBRID); }\n"
"       if (newMapType == \"TERRAIN\")   { map.setMapTypeId(google.maps.MapTypeId.TERRAIN); }\n"
"   }\n"
"   function wmwGetMapType() {\n"
"       var myMapType = map.getMapTypeId();\n"
"       if (myMapType == google.maps.MapTypeId.SATELLITE) { return \"SATELLITE\"; }\n"
"       if (myMapType == google.maps.MapTypeId.ROADMAP )  { return \"ROADMAP\"; }\n"
"       if (myMapType == google.maps.MapTypeId.HYBRID )   { return \"HYBRID\"; }\n"
"       if (myMapType == google.maps.MapTypeId.TERRAIN )  { return \"TERRAIN\"; }\n"
"       return "";\n" // unexpected result
"   }\n"
"   function wmwClearMarkers() {\n"
"       for (var i in markerList) {\n"
"           markerList[i].setMap(null);\n"
"       }\n"
"       markerList = new Object();\n"
"   }\n"
"   function wmwAddMarker(id, lat, lon, setDraggable) {\n"
"       var latlng = new google.maps.LatLng(lat, lon);\n"
"       var marker = new google.maps.Marker({\n"
"           position: latlng,\n"
"           map: map,\n"
"           draggable: setDraggable\n"
"       });\n"
"       google.maps.event.addListener(marker, 'dragend', function() {\n"
"           wmwPostEventString('mm'+id.toString());\n"
"       });\n"
"       markerList[id] = marker;\n"
"   }\n"
"   function wmwGetMarkerPosition(id) {\n"
"       var latlngString;\n"
"       if (markerList[id.toString()]) {\n"
"           latlngString = markerList[id.toString()].getPosition().toUrlValue(12);\n"
"       }\n"
"       return latlngString;\n"
"   }\n"
"   function initialize() {\n"
"       var latlng = new google.maps.LatLng(%1, %2);\n"
"       var myOptions = {\n"
"           zoom: 8,\n"
"           center: latlng,\n"
"           mapTypeId: google.maps.MapTypeId.%3\n"
"       };\n"
"       var mapDiv = document.getElementById(\"map_canvas\");\n"
// "       mapDiv.style.height=\"100%\"\n";
"       map = new google.maps.Map(mapDiv, myOptions);\n"
"       google.maps.event.addListener(map, 'maptypeid_changed', function() {\n"
"           wmwPostEventString('MT'+wmwGetMapType());\n"
"       });\n"
"   }\n"
"</script>\n"
"</head>\n"
"<body onload=\"initialize()\" style=\"padding: 0px; margin: 0px;\">\n"
// "   <div id=\"map_canvas\" style=\"width:400px; height:400px; border: 1px solid black;\"></div>\n"
"   <div id=\"map_canvas\" style=\"width:100%; height:400px;\"></div>\n"
// "<div style=\"display: block;\" id=\"mydiv\">aoeu</div>\n"
"</body>\n"
"</html>\n";

    mapHTMLCode = mapHTMLCode.arg(initialCenter.latString())
                             .arg(initialCenter.lonString())
                             .arg(initialMapType);

    kDebug()<<mapHTMLCode;
    begin();
    write(mapHTMLCode);
    end();
}

bool BGMWidget::eventFilter(QObject* object, QEvent* event)
{
    if (object==d->parent)
    {
        if (event->type()==QEvent::Resize)
        {
            QResizeEvent* const resizeEvent = dynamic_cast<QResizeEvent*>(event);
            if (resizeEvent)
            {
                widget()->resize(resizeEvent->size());
                view()->resize(resizeEvent->size());

                // TODO: the map div does not adjust its height properly if height=100%,
                //       therefore we adjust it manually here
                if (d->isReady)
                {
                    executeScript(QString("document.getElementById(\"map_canvas\").style.height=\"%1px\"").arg(d->parent->height()));
                }
            }
        }
    }
    return false;
}

void BGMWidget::slotHTMLCompleted()
{
    d->isReady = true;
    executeScript(QString("document.getElementById(\"map_canvas\").style.height=\"%1px\"").arg(d->parent->height()));
}

void BGMWidget::khtmlMousePressEvent(khtml::MousePressEvent* e)
{
    scanForJSMessages();
    KHTMLPart::khtmlMousePressEvent(e);
}

void BGMWidget::khtmlMouseReleaseEvent(khtml::MouseReleaseEvent* e)
{
    scanForJSMessages();
    KHTMLPart::khtmlMouseReleaseEvent(e);
}

void BGMWidget::khtmlMouseMoveEvent(khtml::MouseMoveEvent *e)
{
    scanForJSMessages();
    KHTMLPart::khtmlMouseMoveEvent(e);
}

void BGMWidget::scanForJSMessages()
{
    const QString status = jsStatusBarText();

    if (status!="(event)")
        return;

    const QString eventBufferString = executeScript(QString("wmwReadEventStrings();")).toString();
    if (eventBufferString.isEmpty())
        return;

    const QStringList events = eventBufferString.split('|');

    emit(signalHTMLEvents(events));
}

} /* WMW2 */

