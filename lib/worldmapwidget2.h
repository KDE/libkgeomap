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

#include <QWidget>
#include <QStringList>

// local includes

#include "worldmapwidget2_primitives.h"

class KAction;
class KConfigGroup;

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

    void addClusterableMarkers(const WMWMarker::List& markerList);
    WMWMarker getClusterableMarker(const int markerIndex);
    WMWMarker getSingleMarker(const int markerIndex);
    void addSingleMarkers(const WMWMarker::List& markerList);
    void updateMarkers();
    void updateClusters();

    void getColorInfos(const int clusterIndex, QColor *fillColor, QColor *strokeColor,
                       Qt::PenStyle *strokeStyle, QString *labelText, QColor *labelColor) const;

    QString convertZoomToBackendZoom(const QString& someZoom, const QString& targetBackend) const;

public Q_SLOTS:
    void slotZoomIn();
    void slotZoomOut();
    void slotUpdateActionsEnabled();
    void slotClustersNeedUpdating();

Q_SIGNALS:
    void signalSingleMarkersMoved(const QList<int>& markerIndices);
    void signalGroupableMarkersMoved(const QList<int>& markerIndices);

protected:
    void applyCacheToBackend();
    void saveBackendToCache();
    void rebuildConfigurationMenu();

protected Q_SLOTS:
    void slotBackendReady(const QString& backendName);
    void slotChangeBackend(QAction* action);
    void slotBackendZoomChanged(const QString& newZoom);
    void slotClustersMoved(const QIntList& clusterIndices);
    void slotMarkersMoved(const QIntList& markerIndices);

private:
    WMWSharedData* const s;
    WorldMapWidget2Private* const d;
};


} /* WMW2 */

#endif /* WORLDMAPWIDGET2_H */
