/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  world map widget library
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

#ifndef KMAP_H
#define KMAP_H

// Qt includes

#include <QAbstractItemModel>
#include <QWidget>
#include <QStringList>

// local includes

#include "kmap_primitives.h"
#include "libkmap_export.h"

class KAction;
class KConfigGroup;
class QItemSelectionModel;
class QDragEnterEvent;
class QDropEvent;
class QMenu;

namespace KMapIface
{

class AbstractMarkerTiler;
class DragDropHandler;
class WMWSharedData;

class KMAP_EXPORT KMap : public QWidget
{
  Q_OBJECT

public:

    KMap(QWidget* const parent = 0);
    ~KMap();

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
    void setGroupedModel(AbstractMarkerTiler* const markerModel);

    void setEditModeAvailable(const bool state);
    void setDragDropHandler(DragDropHandler* const dragDropHandler);
    QVariant getClusterRepresentativeMarker(const int clusterIndex, const int sortKey);

    void setSortOptionsMenu(QMenu* const sortMenu);
    void setSortKey(const int sortKey);
    QPixmap getDecoratedPixmapForCluster(const int clusterId, const WMWSelectionState* const selectedStateOverride, const int* const countOverride, QPoint* const centerPoint);
    void setThumnailSize(const int newThumbnailSize);
    void setGroupingRadius(const int newGroupingRadius);
    void setEditGroupingRadius(const int newGroupingRadius);
    int getThumbnailSize() const;
    int getUndecoratedThumbnailSize() const;
    void setEditEnabled(const bool state);
    QList<double> selectionCoordinates() const;
    void setSelectionCoordinates(QList<double>& sel);
    bool hasSelection() const; 

public Q_SLOTS:

    void slotZoomIn();
    void slotZoomOut();
    void slotUpdateActionsEnabled();
    void slotClustersNeedUpdating();
    void slotDecreaseThumbnailSize();
    void slotIncreaseThumbnailSize();
    void slotNewSelectionFromMap(const QList<double>& sel);
    void slotSetSelectionMode();
    void slotSetPanMode();
    void slotRemoveCurrentSelection();

Q_SIGNALS:

    void signalAltitudeLookupReady(const KMapIface::WMWAltitudeLookup::List& altitudes);
/*  void signalDisplayMarkersMoved(const QList<QPersistentModelIndex>& indices, 
                                   const KMapIface::WMWGeoCoordinate& coordinates); */
    void signalSpecialMarkersMoved(const QList<QPersistentModelIndex>& indices);
    void signalUngroupedModelChanged(const int index);
    void signalNewSelectionFromMap();
    void signalRemoveCurrentSelection();

public:

    /** Return a string version of LibMarbleWidget release in format "major.minor.patch"
     */
    static QString LibMarbleWidget();

    /** Return a string version of libkmap release
     */
    static QString version();

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

    class KMapPrivate;
    KMapPrivate* const d;
};

} /* namespace KMapIface */

#endif /* KMAP_H */
