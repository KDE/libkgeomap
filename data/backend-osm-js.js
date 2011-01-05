/* ============================================================
 *
 * Date        : 2010-02-05
 * Description : JavaScript part of the OpenStreetMap-backend for WorldMapWidget2
 *
 * Copyright (C) 2010, 2011 by Michael G. Hansen <mike at mghansen dot de>
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
var mapLayer;
var eventBuffer = new Array();
var markerList = new Object();
var clusterList = new Object();
var vectorLayerMarkers;
var layerColors = Array('ff0000', 'ff7f00', 'ffff00', '00ff00', '00ffff');
for (var layerColorIndex in layerColors) {
    eval('var vectorLayersClusters'+layerColors[layerColorIndex]+';');
}

function kmapLonLat2Projection(lonLat) {
    return lonLat.transform(new OpenLayers.Projection('EPSG:4326'), map.getProjectionObject());
}
function kmapLonLatFromProjection(lonLat) {
    return lonLat.clone().transform(map.getProjectionObject(), new OpenLayers.Projection('EPSG:4326'));
}
function kmapLonLat2String(lonLat) {
    return lonLat.lat.toString()+','+lonLat.lon.toString();
}
function kmapProjectedLonLat2String(lonLat) {
    var myLonLat = kmapLonLatFromProjection(lonLat);
    return kmapLonLat2String(myLonLat);
}
function kmapPostEventString(eventString) {
    eventBuffer.push(eventString);
    window.status = '(event)';
}
function kmapReadEventStrings() {
    var eventBufferString = eventBuffer.join('|');
    eventBuffer = new Array();
    // let the application know that there are no more events waiting:
    window.status = '()';
    return eventBufferString;
}
function kmapDebugOut(someString) {
    if (typeof kmapDebugHook == 'function') {
        kmapDebugHook(someString);
    } else {
        kmapPostEventString('do'+someString);
    }
}
function kmapSetZoom(zoomvalue) {
    map.zoomTo(zoomvalue);
}
function kmapGetZoom() {
    return map.getZoom();
}
function kmapZoomIn() {
    map.zoomIn();
}
function kmapZoomOut() {
    map.zoomOut();
}
function kmapSetCenter(lat, lon) {
    var lonLat = new OpenLayers.LonLat(lon, lat);
    map.setCenter(kmapLonLat2Projection(lonLat), 2);
}
function kmapGetCenter() {
    var lonLat = kmapLonLatFromProjection(map.getCenter());
    return kmapLonLat2String(lonLat);
}
function kmapGetBounds() {
    var mapBounds = mapLayer.getExtent();
    mapBounds.transform(map.getProjectionObject(),new OpenLayers.Projection('EPSG:4326'));

    return '(('+mapBounds.bottom+','+mapBounds.left+'),('+mapBounds.top+','+mapBounds.right+'))';
}
function kmapLatLngToPixel(lat, lon) {
    // TODO: do we need to transform the lonlat???
    var myPixel = map.getPixelFromLonLat(kmapLonLat2Projection(new OpenLayers.LonLat(lon, lat)));
    return '('+myPixel.x.toString()+','+myPixel.y.toString()+')';
}
function kmapPixelToLatLng(x, y) {
    // TODO: do we need to transform the lonlat???
    var myLonLat = kmapLonLatFromProjection(map.getLonLatFromPixel(new OpenLayers.Pixel(x, y)));
    return kmapLonLat2String(myLonLat);
}
function kmapClearMarkers() {
    for (var i in markerList) {
        vectorLayerMarkers.removeFeatures(markerList[i]);
        markerList[i].destroy();
    }
    markerList = new Object();
}
function kmapGetMapBounds() {
}
function kmapAddMarker(id, lat, lon, setDraggable) {
    var projectedLonLat = kmapLonLat2Projection(new OpenLayers.LonLat(lon, lat));

    var myVectorMarker = new OpenLayers.Feature.Vector(
            new OpenLayers.Geometry.Point(projectedLonLat.lon, projectedLonLat.lat)
        );
    vectorLayerMarkers.addFeatures(myVectorMarker);
    markerList[id.toString()] = myVectorMarker;
}
function kmapGetMarkerPosition(id) {
    var latlngString;
    if (markerList[id.toString()]) {
        var markerClone = markerList[id.toString()].clone();
        markerClone.geometry.transform(new OpenLayers.Projection("EPSG:900913"), new OpenLayers.Projection("EPSG:4326"));
        var x = markerClone.geometry.x;
        var y = markerClone.geometry.y;
        latlngString = y.toString()+','+x.toString();
    }
    return latlngString;
}
function kmapClearClusters() {
    for (var layerColorIndex in layerColors) {
        eval('vectorLayersClusters'+layerColors[layerColorIndex]+'.removeFeatures(vectorLayersClusters'+layerColors[layerColorIndex]+'.features);');
    }
    for (var i in clusterList) {
        clusterList[i].destroy();
    }
    clusterList = new Object();
}
function kmapAddCluster(id, lat, lon, setDraggable, clusterColor, labelText) {
    var projectedLonLat = kmapLonLat2Projection(new OpenLayers.LonLat(lon, lat));

    var myVectorMarker = new OpenLayers.Feature.Vector(
            new OpenLayers.Geometry.Point(projectedLonLat.lon, projectedLonLat.lat)
            // setting styles per point does not seem to work in KHTML...
//             ,null,
//             new OpenLayers.StyleMap({
//                 externalGraphic : iconFile,
//                 pointRadius: 15
//             })
        );
    eval('vectorLayersClusters'+clusterColor+'.addFeatures(myVectorMarker);');
    clusterList[id.toString()] = myVectorMarker;
}
function kmapGetClusterPosition(id) {
    var latlngString;
    if (clusterList[id.toString()]) {
        var clusterClone = clusterList[id.toString()].clone();
        clusterClone.geometry.transform(new OpenLayers.Projection("EPSG:900913"), new OpenLayers.Projection("EPSG:4326"));
        var x = clusterClone.geometry.x;
        var y = clusterClone.geometry.y;
        latlngString = y.toString()+','+x.toString();
    }
    return latlngString;
}
function kmapWidgetResized(newWidth, newHeight) {
    document.getElementById('map_canvas').style.height=newHeight.toString()+'px';
}
function kmapInitialize() {
    map = new OpenLayers.Map("map_canvas", {
        controls:[
            new OpenLayers.Control.Navigation(),
            new OpenLayers.Control.PanZoomBar(),
            new OpenLayers.Control.Attribution()],
        projection: new OpenLayers.Projection("EPSG:900913"),
        displayProjection: new OpenLayers.Projection("EPSG:4326")
    } );
    mapLayer = new OpenLayers.Layer.OSM.Osmarender("Osmarender");
    map.addLayer(mapLayer);

    for (var layerColorIndex in layerColors) {
        eval('vectorLayersClusters'+layerColors[layerColorIndex]+' = new OpenLayers.Layer.Vector("Vector layer clusters",'+
            '{ styleMap: new OpenLayers.StyleMap({ pointRadius: 15,  externalGraphic : "cluster-circle-'+layerColors[layerColorIndex]+'.png" }) });');
        eval('map.addLayer(vectorLayersClusters'+layerColors[layerColorIndex]+');');
    }

    // create a vector layer to hold the movable markers:
    vectorLayerMarkers = new OpenLayers.Layer.Vector("Vector layer markers",
        {
            styleMap: new OpenLayers.StyleMap({
                externalGraphic : "marker-00ff00.png",
                graphicWidth : 20,
                graphicHeight : 32,
                graphicXOffset : -10,
                graphicYOffset : 0
            })
        });
    map.addLayer(vectorLayerMarkers);

    var dragFeature = new OpenLayers.Control.DragFeature(vectorLayerMarkers);
    dragFeature.onComplete = function(feature, pixel) {
            // get the id of the feature
            // TODO: is there an easier way? Features seem to be able to have ids,
            // but they are not explained in the documentation
            for (id in markerList) {
                if (markerList[id] == feature) {
                    // marker moved
                    kmapPostEventString('mm'+id);
                    return;
                }
            }
            for (id in clusterList) {
                if (clusterList[id] == feature) {
                    // marker moved
                    kmapPostEventString('cm'+id);
                    return;
                }
            }
        };
    map.addControl(dragFeature);
    dragFeature.activate();

    kmapSetCenter(52.0, 6.0);

    map.events.register('moveend', map, function() {
        kmapPostEventString('id');
    } );

    kmapDebugOut('OSM initialize done');
}
