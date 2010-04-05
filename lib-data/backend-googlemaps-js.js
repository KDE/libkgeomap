/* ============================================================
 *
 * Date        : 2010-02-05
 * Description : JavaScript part of the GoogleMaps-backend for WorldMapWidget2
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
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

var mapDiv;
var map;
var eventBuffer = new Array();
var markerList = new Object();
var clusterList = new Object();
var clusterDataList = new Object();
var isInEditMode = false;
var dragMarker;

// ProjectionHelp: http://taapps-javalibs.blogspot.com/2009/10/google-map-v3how-to-use-overlayviews.html
function ProjectionHelper(overlayMap) {
    google.maps.OverlayView.call(this);
    this.setMap(overlayMap);
}
ProjectionHelper.prototype = new google.maps.OverlayView();
ProjectionHelper.prototype.draw = function() {
}
var projectionHelper = null;

function wmwPostEventString(eventString) {
    eventBuffer.push(eventString);
    window.status = '(event)';
}
function wmwReadEventStrings() {
    var eventBufferString = eventBuffer.join('|');
    eventBuffer = new Array();
    // let the application know that there are no more events waiting:
    window.status = '()';
    return eventBufferString;
}
function wmwDebugOut(someString) {
    wmwPostEventString('do'+someString);
}
function wmwSetZoom(zoomvalue) {
    map.setZoom(zoomvalue);
}
function wmwGetZoom() {
    return map.getZoom();
}
function wmwGetMaxZoom() {
    return map.mapTypes[map.getMapTypeId()].maxZoom;
}
function wmwGetMinZoom() {
    var minZoom = map.mapTypes[map.getMapTypeId()].minZoom;
    if (minZoom==null) {
        minZoom = 1;
    }
    return minZoom;
}
function wmwZoomIn() {
    map.setZoom(map.getZoom()+1);
}
function wmwZoomOut() {
    map.setZoom(map.getZoom()-1);
}
function wmwSetCenter(lat, lon) {
    var latlng = new google.maps.LatLng(lat, lon);
    map.setCenter(latlng);
}
function wmwGetCenter() {
    var latlngString = map.getCenter().toUrlValue(12);
    return latlngString;
}
function wmwGetBounds() {
    return map.getBounds().toString();
}
function wmwSetIsInEditMode(state) {
    isInEditMode = state;
}
function wmwLatLngToPixel(lat, lon) {
    //      There is an offset in fromLatLngToDivPixel once the map has been panned
    var latlng = new google.maps.LatLng(lat, lon);
    var myPoint = projectionHelper.getProjection().fromLatLngToDivPixel(latlng);
    var centerPoint = projectionHelper.getProjection().fromLatLngToDivPixel(map.getCenter());
    var centerOffsetX = Math.floor(mapDiv.offsetWidth / 2);
    var centerOffsetY = Math.floor(mapDiv.offsetHeight / 2);
    var pointX = myPoint.x-centerPoint.x+centerOffsetX;
    var pointY = myPoint.y-centerPoint.y+centerOffsetY;
    return new google.maps.Point(pointX, pointY).toString();
//         return projectionHelper.getProjection().fromLatLngToDivPixel(latlng).toString();
}
function wmwPixelToLatLngObject(x, y) {
    //      There is an offset in fromDivPixelToLatLng once the map has been panned
    var centerPoint = projectionHelper.getProjection().fromLatLngToDivPixel(map.getCenter());
    var centerOffsetX = mapDiv.offsetWidth / 2;
    var centerOffsetY = mapDiv.offsetHeight / 2;
    var pointX = x+centerPoint.x-centerOffsetX;
    var pointY = y+centerPoint.y-centerOffsetY;
    var point = new google.maps.Point(pointX, pointY);
    return projectionHelper.getProjection().fromDivPixelToLatLng(point);
}
function wmwPixelToLatLng(x, y) {
    return wmwPixelToLatLngObject(x, y).toUrlValue(12);
}
// parameter: "SATELLITE"/"ROADMAP"/"HYBRID"/"TERRAIN"
function wmwSetMapType(newMapType) {
    if (newMapType == "SATELLITE") { map.setMapTypeId(google.maps.MapTypeId.SATELLITE); }
    if (newMapType == "ROADMAP")   { map.setMapTypeId(google.maps.MapTypeId.ROADMAP); }
    if (newMapType == "HYBRID")    { map.setMapTypeId(google.maps.MapTypeId.HYBRID); }
    if (newMapType == "TERRAIN")   { map.setMapTypeId(google.maps.MapTypeId.TERRAIN); }
}
function wmwGetMapType() {
    var myMapType = map.getMapTypeId();
    if (myMapType == google.maps.MapTypeId.SATELLITE) { return "SATELLITE"; }
    if (myMapType == google.maps.MapTypeId.ROADMAP )  { return "ROADMAP"; }
    if (myMapType == google.maps.MapTypeId.HYBRID )   { return "HYBRID"; }
    if (myMapType == google.maps.MapTypeId.TERRAIN )  { return "TERRAIN"; }
    return "";
}
function wmwSetShowMapTypeControl(state) {
    var myOptions = {
        mapTypeControl: state
    }
    map.setOptions(myOptions);
}
function wmwSetShowNavigationControl(state) {
    var myOptions = {
        navigationControl: state
    }
    map.setOptions(myOptions);
}
function wmwSetShowScaleControl(state) {
    var myOptions = {
        scaleControl: state
    }
    map.setOptions(myOptions);
}
function wmwClearMarkers() {
    for (var i in markerList) {
        markerList[i].setMap(null);
    }
    markerList = new Object();
}
function wmwAddMarker(id, lat, lon, setDraggable) {
    var latlng = new google.maps.LatLng(lat, lon);
    var marker = new google.maps.Marker({
        position: latlng,
        map: map,
        draggable: setDraggable,
        icon: new google.maps.MarkerImage('marker-green.png', new google.maps.Size(20, 32))
    });
    google.maps.event.addListener(marker, 'dragend', function() {
        wmwPostEventString('mm'+id.toString());
    });
    markerList[id] = marker;
}
function wmwGetMarkerPosition(id) {
    var latlngString;
    if (markerList[id.toString()]) {
        latlngString = markerList[id.toString()].getPosition().toUrlValue(12);
    }
    return latlngString;
}
function wmwClearClusters() {
    for (var i in clusterList) {
        clusterList[i].setMap(null);
    }
    clusterList = new Object();
    clusterDataList = new Object();
}
function wmwGetPixmapName(markerCount, markerSelectedCount)
{
    var colorCode;
    if (markerCount>=100)
    {
        colorCode="ff0000";
    }
    else if (markerCount>=50)
    {
        colorCode="ff7f00";
    }
    else if (markerCount>=10)
    {
        colorCode="ffff00";
    }
    else if (markerCount>=2)
    {
        colorCode="00ff00";
    }
    else
    {
        colorCode="00ffff";
    }
    if (markerSelectedCount==markerCount)
    {
        colorCode+="-selected";
    }
    else if (markerSelectedCount>0)
    {
        colorCode+="-someselected";
    }
    return colorCode;
}
function wmwSetClusterPixmap(id, pixmapurl) {
    clusterList[id].setIcon(pixmapurl);
}
function wmwAddCluster(id, lat, lon, setDraggable, markerCount, markerSelectedCount) {
    var latlng = new google.maps.LatLng(lat, lon);
    var clusterIcon;
    var colorCode = wmwGetPixmapName(markerCount, markerSelectedCount);
    if (isInEditMode) {
        clusterIcon = new google.maps.MarkerImage('marker-'+colorCode+'.png', new google.maps.Size(20, 32));
    } else {
        clusterIcon = new google.maps.MarkerImage('cluster-circle-'+colorCode+'.png', new google.maps.Size(30, 30), new google.maps.Point(0,0), new google.maps.Point(15, 15));
    }
    var marker = new google.maps.Marker({
        position: latlng,
        map: map,
        draggable: isInEditMode,
        icon: clusterIcon
    });
    google.maps.event.addListener(marker, 'dragstart', function() {
        var movingClusterData = clusterDataList[id];
        if (movingClusterData.MarkerSelectedCount==0) {
            // no need to change the cluster in any way
            return;
        }
        // at least some items in the cluster are selected. we have to scan all clusters and
        // take their selected markers:
        var newSelectedCount = 0;
        for (var i in clusterList) {
            var clusterData = clusterDataList[i];
            if (clusterData.MarkerSelectedCount>0) {
                newSelectedCount+=clusterData.MarkerSelectedCount;
                var newMarkerCount = clusterData.MarkerCount-clusterData.MarkerSelectedCount;

                if (i!=id) {
                    var colorCode = wmwGetPixmapName(newMarkerCount, 0);
                    var clusterIcon = new google.maps.MarkerImage('marker-'+colorCode+'.png', new google.maps.Size(20, 32));
                    clusterList[i].setOptions({ icon: clusterIcon, title: newMarkerCount.toString() });
                }
            }
        }
        // adjust the moving marker
        var colorCode = wmwGetPixmapName(newSelectedCount, newSelectedCount);
        var clusterIcon = new google.maps.MarkerImage('marker-'+colorCode+'.png', new google.maps.Size(20, 32));
        clusterList[id].setOptions({ icon: clusterIcon, title: newSelectedCount.toString()});

        // create a leftover-marker:
        var leftOverMarkerCount=movingClusterData.MarkerCount-movingClusterData.MarkerSelectedCount;
        if (leftOverMarkerCount>0) {
            var colorCode = wmwGetPixmapName(leftOverMarkerCount, 0);
            var clusterIcon = new google.maps.MarkerImage('marker-'+colorCode+'.png', new google.maps.Size(20, 32));
            var leftOverMarker = new google.maps.Marker({
                position: latlng,
                map: map,
                icon: clusterIcon,
                title: leftOverMarkerCount.toString()
            });
            clusterList[-1]=leftOverMarker;
        }
    });
    google.maps.event.addListener(marker, 'dragend', function() {
        wmwPostEventString('cm'+id.toString());
    });
    google.maps.event.addListener(marker, 'click', function() {
        wmwPostEventString('cc'+id.toString());
    });
    clusterList[id] = marker;
    var clusterData = new Object();
    clusterData["MarkerCount"]=markerCount;
    clusterData["MarkerSelectedCount"]=markerSelectedCount;
    clusterDataList[id]=clusterData;
}
function wmwGetClusterPosition(id) {
    var latlngString;
    if (clusterList[id.toString()]) {
        latlngString = clusterList[id.toString()].getPosition().toUrlValue(12);
    }
    return latlngString;
}
function wmwWidgetResized(newWidth, newHeight) {
    document.getElementById('map_canvas').style.height=newHeight.toString()+'px';
}
function wmwRemoveDragMarker() {
    if (dragMarker) {
        dragMarker.setMap(null);
        dragMarker = null;
    }
}
function wmwMoveDragMarker(x, y) {
    if (dragMarker) {
        dragMarker.setPosition(wmwPixelToLatLngObject(x ,y));
    }
}
function wmwSetDragMarker(x, y, markerCount, markerSelectedCount) {
    wmwRemoveDragMarker();
    var latlng = wmwPixelToLatLngObject(x, y);
    var colorCode = wmwGetPixmapName(markerCount, markerSelectedCount);
    var clusterIcon = new google.maps.MarkerImage('marker-'+colorCode+'.png', new google.maps.Size(20, 32));
    dragMarker = new google.maps.Marker({
        position: latlng,
        map: map,
        icon: clusterIcon
    });
}
function initialize() {
    var latlng = new google.maps.LatLng(52.0, 6.0);
    var myOptions = {
        zoom: 8,
        center: latlng,
        mapTypeId: google.maps.MapTypeId.ROADMAP
    };
    mapDiv = document.getElementById("map_canvas");
    //       mapDiv.style.height="100%"
    map = new google.maps.Map(mapDiv, myOptions);
    google.maps.event.addListener(map, 'maptypeid_changed', function() {
        wmwPostEventString('MT'+wmwGetMapType());
    });
    //  these are too heavy on the performance. monitor 'idle' event only for now:
    //       google.maps.event.addListener(map, 'bounds_changed', function() {
    //           wmwPostEventString('MB');
    //       });
    //       google.maps.event.addListener(map, 'zoom_changed', function() {
    //           wmwPostEventString('ZC');
    //       });
    google.maps.event.addListener(map, 'idle', function() {
        wmwPostEventString('id');
    });
    // source: http://taapps-javalibs.blogspot.com/2009/10/google-map-v3how-to-use-overlayviews.html
    projectionHelper = new ProjectionHelper(map);
}

