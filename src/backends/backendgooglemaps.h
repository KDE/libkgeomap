/** ===========================================================
 * @file
 *
 * This file is a part of KDE project
 *
 *
 * @date   2009-12-01
 * @brief  Google-Maps-backend for KGeoMap
 *
 * @author Copyright (C) 2009-2010, 2014 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2014 by Justus Schwartz
 *         <a href="mailto:justus at gmx dot li">justus at gmx dot li</a>
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

#ifndef BACKEND_MAP_GOOGLEMAPS_H
#define BACKEND_MAP_GOOGLEMAPS_H

// local includes

#include "mapbackend.h"
#include "tracks.h"

namespace KGeoMap
{

class BackendGoogleMaps : public MapBackend
{
    Q_OBJECT

public:

    explicit BackendGoogleMaps(const QExplicitlySharedDataPointer<KGeoMapSharedData>& sharedData, QObject* const parent = nullptr);
    ~BackendGoogleMaps() override;

    QString backendName() const override;
    QString backendHumanName() const override;
    QWidget* mapWidget() override;
    void releaseWidget(KGeoMapInternalWidgetInfo* const info) override;
    void mapWidgetDocked(const bool state) override;

    GeoCoordinates getCenter() const override;
    void setCenter(const GeoCoordinates& coordinate) override;

    bool isReady() const override;

    void zoomIn() override;
    void zoomOut() override;

    void saveSettingsToGroup(KConfigGroup* const group) override;
    void readSettingsFromGroup(const KConfigGroup* const group) override;

    void addActionsToConfigurationMenu(QMenu* const configurationMenu) override;

    void updateMarkers() override;
    void updateClusters() override;

    bool screenCoordinates(const GeoCoordinates& coordinates, QPoint* const point) override;
    bool geoCoordinates(const QPoint& point, GeoCoordinates* const coordinates) const override;
    QSize mapSize() const override;

    void setZoom(const QString& newZoom) override;
    QString getZoom() const override;

    int getMarkerModelLevel() override;
    GeoCoordinates::PairList getNormalizedBounds() override;

//     virtual void updateDragDropMarker(const QPoint& pos, const KGeoMapDragData* const dragData);
//     virtual void updateDragDropMarkerPosition(const QPoint& pos);

    void updateActionAvailability() override;

    QString getMapType() const;
    void setMapType(const QString& newMapType);
    void setShowMapTypeControl(const bool state);
    void setShowScaleControl(const bool state);
    void setShowNavigationControl(const bool state);

    void regionSelectionChanged() override;
    void mouseModeChanged() override;

    void centerOn(const Marble::GeoDataLatLonBox& latLonBox, const bool useSaneZoomLevel) override;
    void setActive(const bool state) override;

public Q_SLOTS:

    void slotClustersNeedUpdating() override;
    void slotThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap) override;
    void slotUngroupedModelChanged(const int mindex);
  
protected:

    bool eventFilter(QObject* object, QEvent* event) override;
    void createActions();
    void setClusterPixmap(const int clusterId, const QPoint& centerPoint, const QPixmap& clusterPixmap);
    void setMarkerPixmap(const int modelId, const int markerId, const QPoint& centerPoint, const QPixmap& markerPixmap);
    void setMarkerPixmap(const int modelId, const int markerId, const QPoint& centerPoint, const QSize& iconSize, const QUrl& iconUrl);
    void storeTrackChanges(const TrackManager::TrackChanges trackChanges);

private Q_SLOTS:

    void slotHTMLInitialized();
    void slotMapTypeActionTriggered(QAction* action);
    void slotHTMLEvents(const QStringList& eventStrings);
    void slotFloatSettingsTriggered(QAction* action);
    void slotSelectionHasBeenMade(const KGeoMap::GeoCoordinates::Pair& searchCoordinates);
    void slotTrackManagerChanged() override;
    void slotTracksChanged(const QList<TrackManager::TrackChanges> trackChanges);
    void slotTrackVisibilityChanged(const bool newState);

private:

    void updateZoomMinMaxCache();
    static void deleteInfoFunction(KGeoMapInternalWidgetInfo* const info);
    void addPointsToTrack(const quint64 trackId, TrackManager::TrackPoint::List const& track, const int firstPoint, const int nPoints);
  
private:

    class Private;
    Private* const d;
};

} /* namespace KGeoMap */

#endif /* BACKEND_MAP_GOOGLEMAPS_H */
