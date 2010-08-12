 /** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  Google-Maps-backend for KMap
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

#ifndef BACKEND_GOOGLEMAPS_H
#define BACKEND_GOOGLEMAPS_H

// local includes

#include "map-backend.h"

// Marble Widget include

#include <marble/global.h>
#include <marble/GeoDataLatLonBox.h>

namespace KMapIface
{

class BackendGoogleMaps : public MapBackend
{
    Q_OBJECT

public:

    BackendGoogleMaps(const QExplicitlySharedDataPointer<WMWSharedData>& sharedData, QObject* const parent = 0);
    virtual ~BackendGoogleMaps();

    virtual QString backendName() const;
    virtual QString backendHumanName() const;
    virtual QWidget* mapWidget() const;

    virtual WMWGeoCoordinate getCenter() const;
    virtual void setCenter(const WMWGeoCoordinate& coordinate);

    virtual bool isReady() const;

    virtual void zoomIn();
    virtual void zoomOut();

    virtual void saveSettingsToGroup(KConfigGroup* const group);
    virtual void readSettingsFromGroup(const KConfigGroup* const group);

    virtual void addActionsToConfigurationMenu(QMenu* const configurationMenu);

    virtual void updateMarkers();
    virtual void updateClusters();

    virtual bool screenCoordinates(const WMWGeoCoordinate& coordinates, QPoint* const point);
    virtual bool geoCoordinates(const QPoint& point, WMWGeoCoordinate* const coordinates) const;
    virtual QSize mapSize() const;

    virtual void setZoom(const QString& newZoom);
    virtual QString getZoom() const;

    virtual int getMarkerModelLevel();
    virtual WMWGeoCoordinate::PairList getNormalizedBounds();

//     virtual void updateDragDropMarker(const QPoint& pos, const WMWDragData* const dragData);
//     virtual void updateDragDropMarkerPosition(const QPoint& pos);

    virtual void updateActionAvailability();

    QString getMapType() const;
    void setMapType(const QString& newMapType);
    void setShowMapTypeControl(const bool state);
    void setShowScaleControl(const bool state);
    void setShowNavigationControl(const bool state);

    virtual void setSelectionRectangle(const QList<double>& searchCoordinates);
    virtual QList<qreal> getSelectionRectangle();
    virtual void removeSelectionRectangle();
    virtual void mouseModeChanged(MouseMode mouseMode);

    virtual void setSelectionStatus(const bool status);
    virtual void centerOn(const Marble::GeoDataLatLonBox& latLonBox);

public Q_SLOTS:

    virtual void slotClustersNeedUpdating();
    virtual void slotThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap);
    void slotUngroupedModelChanged(const int mindex);
    void slotSelectionHasBeenMade(const QList<qreal>& searchCoordinates);

protected:

    bool eventFilter(QObject* object, QEvent* event);
    void createActions();
    void setClusterPixmap(const int clusterId, const QPoint& centerPoint, const QPixmap& clusterPixmap);
    void setMarkerPixmap(const int modelId, const int markerId, const QPoint& centerPoint, const QPixmap& markerPixmap);

private Q_SLOTS:

    void slotHTMLInitialized();
    void slotMapTypeActionTriggered(QAction* action);
    void slotHTMLEvents(const QStringList& eventStrings);
    void slotFloatSettingsTriggered(QAction* action);

private:

    void loadInitialHTML();
    void updateZoomMinMaxCache();

private:

    class BackendGoogleMapsPrivate;
    BackendGoogleMapsPrivate* const d;
};

} /* namespace KMapIface */

#endif /* BACKEND_GOOGLEMAPS_H */
