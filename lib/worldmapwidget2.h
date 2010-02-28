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

class KAction;
class KConfigGroup;
class QItemSelectionModel;

namespace WMW2 {

class WorldMapWidget2Private;

class WorldMapWidget2 : public QWidget
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

    void updateMarkers();
    void updateClusters();

    void getColorInfos(const int clusterIndex, QColor *fillColor, QColor *strokeColor,
                       Qt::PenStyle *strokeStyle, QString *labelText, QColor *labelColor,
                                    const WMWSelectionState* const overrideSelection = 0,
                                    const int* const overrideCount = 0) const;

    QString convertZoomToBackendZoom(const QString& someZoom, const QString& targetBackend) const;
    bool queryAltitudes(const WMWAltitudeLookup::List& queryItems, const QString& backendName = "");

    void setSpecialMarkersModel(QAbstractItemModel* const specialMarkersModel, const int coordinatesRole);
    void setDisplayMarkersModel(QAbstractItemModel* const displayMarkersModel, const int coordinatesRole, QItemSelectionModel* const selectionModel = 0);

public Q_SLOTS:
    void slotZoomIn();
    void slotZoomOut();
    void slotUpdateActionsEnabled();
    void slotClustersNeedUpdating();

Q_SIGNALS:
    void signalAltitudeLookupReady(const WMW2::WMWAltitudeLookup::List& altitudes);
    void signalDisplayMarkersMoved(const QList<QPersistentModelIndex>& indices);
    void signalSpecialMarkersMoved(const QList<QPersistentModelIndex>& indices);

protected:
    void applyCacheToBackend();
    void saveBackendToCache();
    void rebuildConfigurationMenu();

protected Q_SLOTS:
    void slotBackendReady(const QString& backendName);
    void slotChangeBackend(QAction* action);
    void slotBackendZoomChanged(const QString& newZoom);
    void slotClustersMoved(const QIntList& clusterIndices);
    void slotGroupModeChanged(QAction* triggeredAction);

private:
    const QExplicitlySharedDataPointer<WMWSharedData> s;
    WorldMapWidget2Private* const d;
};


} /* WMW2 */

#endif /* WORLDMAPWIDGET2_H */
