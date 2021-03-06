/** ===========================================================
 * @file
 *
 * This file is a part of KDE project
 *
 *
 * @date   2009-12-08
 * @brief  Marble-backend for KGeoMap
 *
 * @author Copyright (C) 2009-2011, 2014 by Michael G. Hansen
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

#ifndef BACKEND_MAP_MARBLE_H
#define BACKEND_MAP_MARBLE_H

// local includes

#include "mapbackend.h"
#include "tracks.h"

/// @cond false
namespace Marble
{
    class GeoPainter;
}
/// @endcond

namespace KGeoMap
{

class BackendMarble : public MapBackend
{
    Q_OBJECT

public:

    explicit BackendMarble(const QExplicitlySharedDataPointer<KGeoMapSharedData>& sharedData, QObject* const parent = nullptr);
    ~BackendMarble() override;

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

    QString getMapTheme() const;
    void setMapTheme(const QString& newMapTheme);

    QString getProjection() const;
    void setProjection(const QString& newProjection);

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

    void marbleCustomPaint(Marble::GeoPainter* painter);
    void setShowCompass(const bool state);
    void setShowOverviewMap(const bool state);
    void setShowScaleBar(const bool state);

    void regionSelectionChanged() override;
    void mouseModeChanged() override;

    void centerOn(const Marble::GeoDataLatLonBox& box, const bool useSaneZoomLevel) override;
    void setActive(const bool state) override;

public Q_SLOTS:

    void slotClustersNeedUpdating() override;
    void slotThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap) override;
    void slotUngroupedModelChanged(const int index);
    void slotTrackManagerChanged() override;

protected:

    bool eventFilter(QObject* object, QEvent* event) override;
    void createActions();
    bool findSnapPoint(const QPoint& actualPoint, QPoint* const snapPoint, GeoCoordinates* const snapCoordinates, QPair<int, QModelIndex>* const snapTargetIndex);
    void GeoPainter_drawPixmapAtCoordinates(Marble::GeoPainter* const painter, const QPixmap& pixmap, const GeoCoordinates& coordinates, const QPoint& basePoint);
    void drawSearchRectangle(Marble::GeoPainter* const painter, const GeoCoordinates::Pair& searchRectangle, const bool isOldRectangle);
    void applyCacheToWidget();

    static void deleteInfoFunction(KGeoMapInternalWidgetInfo* const info);

protected Q_SLOTS:

    void slotMapThemeActionTriggered(QAction* action);
    void slotProjectionActionTriggered(QAction* action);
    void slotFloatSettingsTriggered(QAction* action);
    void slotMarbleZoomChanged();
    void slotTracksChanged(const QList<TrackManager::TrackChanges> trackChanges);
    void slotScheduleUpdate();

private:

    class Private;
    Private* const d;
};

} /* namespace KGeoMap */

#endif /* BACKEND_MAP_MARBLE_H */
