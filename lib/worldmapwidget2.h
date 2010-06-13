/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : WorldMapWidget2
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

#ifndef WORLDMAPWIDGET2_H
#define WORLDMAPWIDGET2_H

// Qt includes

#include <QAbstractItemModel>
#include <QWidget>
#include <QStringList>

// local includes

#include "worldmapwidget2_primitives.h"
#include "worldmapwidget2_export.h"

class KAction;
class KConfigGroup;
class QItemSelectionModel;
class QDragEnterEvent;
class QDropEvent;
class QMenu;

namespace WMW2 {

class WorldMapWidget2Private;
class DragDropHandler;

class WORLDMAPWIDGET2_EXPORT WorldMapWidget2 : public QWidget
{
Q_OBJECT

public:
    WorldMapWidget2(QWidget* const parent = 0);
    ~WorldMapWidget2();

    QStringList availableBackends() const;
    bool setBackend(const QString& backendName);

    WMWGeoCoordinate getCenter() const;
    void setCenter(const WMWGeoCoordinate& coordinate);

    void setZoom(const QString& newZoom);
    QString getZoom();

    void saveSettingsToGroup(KConfigGroup* const group);
    void readSettingsFromGroup(const KConfigGroup* const group);

    KAction* getControlAction(const QString& actionName);
    QWidget* getControlWidget();
    void addWidgetToControlWidget(QWidget* const newWidget);

    void updateMarkers();
    void updateClusters();
    void markClustersAsDirty();

    void getColorInfos(const int clusterIndex, QColor *fillColor, QColor *strokeColor,
                       Qt::PenStyle *strokeStyle, QString *labelText, QColor *labelColor,
                                    const WMWSelectionState* const overrideSelection = 0,
                                    const int* const overrideCount = 0) const;

    void getColorInfos(const WMWSelectionState selectionState,
                       const int nMarkers,
                       QColor *fillColor, QColor *strokeColor,
                       Qt::PenStyle *strokeStyle, QString *labelText, QColor *labelColor) const;

    QString convertZoomToBackendZoom(const QString& someZoom, const QString& targetBackend) const;
    bool queryAltitudes(const WMWAltitudeLookup::List& queryItems, const QString& backendName = "");

    void addUngroupedModel(WMWModelHelper* const modelHelper);
    void setDisplayMarkersModel(QAbstractItemModel* const displayMarkersModel, const int coordinatesRole, QItemSelectionModel* const selectionModel = 0);

    void setEditModeAvailable(const bool state);
    void setDragDropHandler(DragDropHandler* const dragDropHandler);
    QVariant getClusterRepresentativeMarker(const int clusterIndex, const int sortKey);
    void setRepresentativeChooser(WMWRepresentativeChooser* const chooser);
    void setDoUpdateMarkerCoordinatesInModel(const bool doIt);

    void setSortOptionsMenu(QMenu* const sortMenu);
    void setSortKey(const int sortKey);
    QPixmap getDecoratedPixmapForCluster(const int clusterId, const WMWSelectionState* const selectedStateOverride, const int* const countOverride, QPoint* const centerPoint);
    void setThumnailSize(const int newThumbnailSize);
    void setGroupingRadius(const int newGroupingRadius);
    void setEditGroupingRadius(const int newGroupingRadius);
    int getThumbnailSize() const;
    int getUndecoratedThumbnailSize() const;
    void setEditEnabled(const bool state);

public Q_SLOTS:
    void slotZoomIn();
    void slotZoomOut();
    void slotUpdateActionsEnabled();
    void slotClustersNeedUpdating();
    void slotDecreaseThumbnailSize();
    void slotIncreaseThumbnailSize();

Q_SIGNALS:
    void signalAltitudeLookupReady(const WMW2::WMWAltitudeLookup::List& altitudes);
    void signalDisplayMarkersMoved(const QList<QPersistentModelIndex>& indices, const WMW2::WMWGeoCoordinate& coordinates);
    void signalSpecialMarkersMoved(const QList<QPersistentModelIndex>& indices);
    void signalUngroupedModelChanged(const int index);

protected:
    void applyCacheToBackend();
    void saveBackendToCache();
    void rebuildConfigurationMenu();
    void dropEvent(QDropEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void createActions();
    void createActionsForBackendSelection();

protected Q_SLOTS:
    void slotBackendReady(const QString& backendName);
    void slotChangeBackend(QAction* action);
    void slotBackendZoomChanged(const QString& newZoom);
    void slotClustersMoved(const QIntList& clusterIndices, const QPair<int, QModelIndex>& snapTarget);
    void slotClustersClicked(const QIntList& clusterIndices);
    void slotGroupModeChanged(QAction* triggeredAction);
    void slotRequestLazyReclustering();
    void slotLazyReclusteringRequestCallBack();
    void slotItemDisplaySettingsChanged();
    void slotUngroupedModelChanged();

private:
    const QExplicitlySharedDataPointer<WMWSharedData> s;
    WorldMapWidget2Private* const d;
};


} /* WMW2 */

#endif /* WORLDMAPWIDGET2_H */
