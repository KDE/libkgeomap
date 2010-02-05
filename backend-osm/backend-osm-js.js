/* ============================================================
 *
 * Date        : 2010-02-05
 * Description : JavaScript part of the OpenStreetMap-backend for WorldMapWidget2
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
var vectorLayer;

function wmwLonLat2Projection(lonLat) {
    return lonLat.transform(new OpenLayers.Projection('EPSG:4326'), map.getProjectionObject());
}
function wmwLonLatFromProjection(lonLat) {
    return lonLat.transform(map.getProjectionObject(), new OpenLayers.Projection('EPSG:4326'));
}
function wmwLonLat2String(lonLat) {
    return lonLat.lat.toString()+','+lonLat.lon.toString();
}
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
    if (typeof wmwDebugHook == 'function') {
        wmwDebugHook(someString);
    } else {
        wmwPostEventString('do'+someString);
    }
}
function wmwSetZoom(zoomvalue) {
    map.setZoom(zoomvalue);
}
function wmwGetZoom() {
    return map.getZoom();
}
function wmwZoomIn() {
    map.zoomIn();
}
function wmwZoomOut() {
    map.zoomOut();
}
function wmwSetCenter(lat, lon) {
    var lonLat = new OpenLayers.LonLat(lon, lat);
    map.setCenter(wmwLonLat2Projection(lonLat), 2);
}
function wmwGetCenter() {
    var lonLat = wmwLonLatFromProjection(map.getCenter());
    return wmwLonLat2String(lonLat);
}
function wmwLatLngToPixel(lat, lon) {
    // TODO: do we need to transform the lonlat???
    var myPixel = map.getPixelFromLonLat(wmwLonLat2Projection(new OpenLayers.LonLat(lon, lat)));
    return '('+myPixel.x.toString()+','+myPixel.y.toString()+')';
}
function wmwPixelToLatLng(x, y) {
    // TODO: do we need to transform the lonlat???
    var myLonLat = wmwLonLatFromProjection(map.getLonLatFromPixel(new OpenLayers.Pixel(x, y)));
    return wmwLonLat2String(myLonLat);
}
function wmwClearMarkers() {
    for (var i in markerList) {
        vectorLayer.removeFeatures(markerList[i]);
        markerList[i].destroy();
    }
    markerList = new Object();
}
function wmwGetMapBounds() {
}
function wmwAddMarker(id, lat, lon, setDraggable) {
    var projectedLonLat = wmwLonLat2Projection(new OpenLayers.LonLat(lon, lat));

    var myVectorMarker = new OpenLayers.Feature.Vector(
            new OpenLayers.Geometry.Point(projectedLonLat.lon, projectedLonLat.lat)
        );
    vectorLayer.addFeatures(myVectorMarker);
    markerList[id.toString()] = myVectorMarker;
}
//     function wmwGetMarkerPosition(id) {
//         var latlngString;
//         if (markerList[id.toString()]) {
//             wmwDebugOut(id.toString());
//             var markerClone = markerList[id.toString()].clone();
//             wmwDebugOut(markerClone.geometry.toShortString());
//             markerClone.geometry.transform(new OpenLayers.Projection("EPSG:4326"), new OpenLayers.Projection("EPSG:900913"));
//             var x = markerClone.geometry.x;
//             var y = markerClone.geometry.y;
//             wmwDebugOut(x.toString()+','+y.toString());
// //             wmwDebugOut(wmwLatLngToPixel(x,y));
//             latlngString = wmwLonLat2String(new OpenLayers.LonLat(x, y));
//             wmwDebugOut('latLngString:'+latLngString);
//         }
//         return latlngString;
//     }
function wmwClearClusters() {
    for (var i in clusterList) {
        vectorLayer.removeFeatures(clusterList[i]);
        clusterList[i].destroy();
    }
    clusterList = new Object();
}
function wmwAddCluster(id, lat, lon, setDraggable) {
    var projectedLonLat = wmwLonLat2Projection(new OpenLayers.LonLat(lon, lat));

    var myVectorMarker = new OpenLayers.Feature.Vector(
            new OpenLayers.Geometry.Point(projectedLonLat.lon, projectedLonLat.lat)
        );
    vectorLayer.addFeatures(myVectorMarker);
    clusterList[id.toString()] = myVectorMarker;
}
//   function wmwGetClusterPosition(id) {
//       var latlngString;
//       if (clusterList[id.toString()]) {
//           latlngString = clusterList[id.toString()].getPosition().toUrlValue(12);
//       }
//       return latlngString;
//   }
function wmwWidgetResized(newWidth, newHeight) {
    document.getElementById('map_canvas').style.height=newHeight.toString()+'px';
}
function initialize() {
    map = new OpenLayers.Map("map_canvas", {
        controls:[
            new OpenLayers.Control.Navigation(),
            new OpenLayers.Control.PanZoomBar(),
            new OpenLayers.Control.Attribution()],
        projection: new OpenLayers.Projection("EPSG:900913"),
        displayProjection: new OpenLayers.Projection("EPSG:4326")
    } );
    layerTilesAtHome = new OpenLayers.Layer.OSM.Osmarender("Osmarender");
    map.addLayer(layerTilesAtHome);

    // create a vector layer to hold the movable markers:
    vectorLayer = new OpenLayers.Layer.Vector("Vector layer", {
            // no options yet
        });
    map.addLayer(vectorLayer);

    var dragFeature = new OpenLayers.Control.DragFeature(vectorLayer);
    dragFeature.onComplete = function(feature, pixel) {
            // get the id of the feature
            // TODO: is there an easier way? Features seem to be able to have ids,
            // but they are not explained in the documentation
            for (id in markerList) {
                if (markerList[id] == feature) {
                    // marker moved
                    wmwPostEventString('mm'+id);
                    return;
                }
            }
            for (id in clusterList) {
                if (clusterList[id] == feature) {
                    // marker moved
                    wmwPostEventString('cm'+id);
                    return;
                }
            }
        };
    map.addControl(dragFeature);
    dragFeature.activate();

    wmwSetCenter(52.0, 6.0);

    map.events.register('moveend', map, function() {
        wmwPostEventString('id');
    } );

    wmwDebugOut('OSM initialize done');
}
