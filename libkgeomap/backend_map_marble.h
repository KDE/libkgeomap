/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-08
 * @brief  Marble-backend for KMap
 *
 * @author Copyright (C) 2009-2011 by Michael G. Hansen
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

#ifndef BACKEND_MAP_MARBLE_H
#define BACKEND_MAP_MARBLE_H

// local includes

#include "backend_map.h"

/// @cond false
namespace Marble
{
    class GeoPainter;
}
/// @endcond

namespace KMap
{

class BackendMarble : public MapBackend
{
    Q_OBJECT

public:

    explicit BackendMarble(const QExplicitlySharedDataPointer<KMapSharedData>& sharedData, QObject* const parent = 0);
    virtual ~BackendMarble();

    virtual QString backendName() const;
    virtual QString backendHumanName() const;
    virtual QWidget* mapWidget();
    virtual void releaseWidget(KMapInternalWidgetInfo* const info);
    virtual void mapWidgetDocked(const bool state);

    virtual GeoCoordinates getCenter() const;
    virtual void setCenter(const GeoCoordinates& coordinate);

    virtual bool isReady() const;

    virtual void zoomIn();
    virtual void zoomOut();

    virtual void saveSettingsToGroup(KConfigGroup* const group);
    virtual void readSettingsFromGroup(const KConfigGroup* const group);

    virtual void addActionsToConfigurationMenu(QMenu* const configurationMenu);

    virtual void updateMarkers();
    virtual void updateClusters();

    QString getMapTheme() const;
    void setMapTheme(const QString& newMapTheme);

    QString getProjection() const;
    void setProjection(const QString& newProjection);

    virtual bool screenCoordinates(const GeoCoordinates& coordinates, QPoint* const point);
    virtual bool geoCoordinates(const QPoint& point, GeoCoordinates* const coordinates) const;
    virtual QSize mapSize() const;

    virtual void setZoom(const QString& newZoom);
    virtual QString getZoom() const;

    virtual int getMarkerModelLevel();
    virtual GeoCoordinates::PairList getNormalizedBounds();

//     virtual void updateDragDropMarker(const QPoint& pos, const KMapDragData* const dragData);
//     virtual void updateDragDropMarkerPosition(const QPoint& pos);

    virtual void updateActionAvailability();

    void marbleCustomPaint(Marble::GeoPainter* painter);
    void setShowCompass(const bool state);
    void setShowOverviewMap(const bool state);
    void setShowScaleBar(const bool state);

    virtual void regionSelectionChanged();
    virtual void mouseModeChanged();

    virtual void centerOn(const Marble::GeoDataLatLonBox& box, const bool useSaneZoomLevel); 
    virtual void setActive(const bool state);

public Q_SLOTS:

    virtual void slotClustersNeedUpdating();
    virtual void slotThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap);
    void slotUngroupedModelChanged(const int index);

protected:

    bool eventFilter(QObject* object, QEvent* event);
    void createActions();
    bool findSnapPoint(const QPoint& actualPoint, QPoint* const snapPoint, GeoCoordinates* const snapCoordinates, QPair<int, QModelIndex>* const snapTargetIndex);
    void GeoPainter_drawPixmapAtCoordinates(Marble::GeoPainter* const painter, const QPixmap& pixmap, const GeoCoordinates& coordinates, const QPoint& basePoint);
    void drawSearchRectangle(Marble::GeoPainter* const painter, const GeoCoordinates::Pair& searchRectangle, const bool isOldRectangle);
    void applyCacheToWidget();

    static void deleteInfoFunction(KMapInternalWidgetInfo* const info);

protected Q_SLOTS:

    void slotMapThemeActionTriggered(QAction* action);
    void slotProjectionActionTriggered(QAction* action);
    void slotFloatSettingsTriggered(QAction* action);
    void slotMarbleZoomChanged();

private:

    class BackendMarblePrivate;
    BackendMarblePrivate* const d;
};

} /* namespace KMap */

#endif /* BACKEND_MAP_MARBLE_H */
