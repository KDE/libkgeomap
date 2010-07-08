/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  OpenStreetMap-backend for WorldMapWidget2
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
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

#ifndef BACKEND_OSM_H
#define BACKEND_OSM_H

// local includes

#include "map-backend.h"

namespace KMapIface
{

class BackendOSMPrivate;

class BackendOSM : public MapBackend
{
Q_OBJECT

public:
    BackendOSM(const QExplicitlySharedDataPointer<WMWSharedData>& sharedData, QObject* const parent = 0);
    virtual ~BackendOSM();

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

public Q_SLOTS:
    virtual void slotClustersNeedUpdating();

private Q_SLOTS:
    void slotHTMLInitialized();
    void updateActionsEnabled();
    void slotHTMLEvents(const QStringList& eventStrings);

private:
    BackendOSMPrivate* const d;
    void loadInitialHTML();
};

} /* KMapIface */

#endif /* BACKEND_OSM_H */

