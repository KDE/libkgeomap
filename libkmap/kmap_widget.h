/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  world map widget library
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

#ifndef KMAP_WIDGET_H
#define KMAP_WIDGET_H

// Qt includes

#include <QtGui/QWidget>
#include <QtCore/QStringList>

// local includes

#include "kmap_primitives.h"
#include "libkmap_export.h"

class KAction;
class KConfigGroup;
class QDragEnterEvent;
class QDropEvent;
class QMenu;

namespace KMap
{

class AbstractMarkerTiler;
class DragDropHandler;
class KMapSharedData;
class ModelHelper;

class KMAP_EXPORT KMapWidget : public QWidget
{
  Q_OBJECT

public:

    KMapWidget(QWidget* const parent = 0);
    ~KMapWidget();

    void saveSettingsToGroup(KConfigGroup* const group);
    void readSettingsFromGroup(const KConfigGroup* const group);

    /// @name Data
    //@{
    void addUngroupedModel(ModelHelper* const modelHelper);
    void removeUngroupedModel(ModelHelper* const modelHelper);
    void setGroupedModel(AbstractMarkerTiler* const markerModel);
    void setDragDropHandler(DragDropHandler* const dragDropHandler);
    //@}

    /// @name UI setup
    //@{
    KAction* getControlAction(const QString& actionName);
    QWidget* getControlWidget();
    void addWidgetToControlWidget(QWidget* const newWidget);
    void setSortOptionsMenu(QMenu* const sortMenu);
    void setMouseMode(const MouseModes mouseMode);
    void setAvailableMouseModes(const MouseModes mouseModes);
    void setVisibleMouseModes(const MouseModes mouseModes);
    void setAllowModifications(const bool state);
    void setActive(const bool state);
    bool getActiveState();
    bool getStickyModeState() const;
    void setStickyModeState(const bool state);
    void setVisibleExtraActions(const ExtraActions actions);
    void setEnabledExtraActions(const ExtraActions actions);
    //@}

    /// @name Map related functions
    //@{
    QStringList availableBackends() const;
    bool setBackend(const QString& backendName);

    GeoCoordinates getCenter() const;
    void setCenter(const GeoCoordinates& coordinate);

    void setZoom(const QString& newZoom);
    QString getZoom();

    void adjustBoundariesToGroupedMarkers(const bool useSaneZoomLevel = true);
    void refreshMap();
    //@}

    /// @name Appearance
    //@{
    void setSortKey(const int sortKey);
    void setThumnailSize(const int newThumbnailSize);
    void setThumbnailGroupingRadius(const int newGroupingRadius);
    void setMarkerGroupingRadius(const int newGroupingRadius);
    int getThumbnailSize() const;
    int getUndecoratedThumbnailSize() const;
    void setShowThumbnails(const bool state);
    //@}

    /// @name Region selection
    //@{
    void setRegionSelection(const GeoCoordinates::Pair& region);
    GeoCoordinates::Pair getRegionSelection();
    void clearRegionSelection();
    //@}

    /// @name Miscellaneous
    //@{
    bool queryAltitudes(const KMapAltitudeLookup::List& queryItems, const QString& backendName = QLatin1String( "" ));
    //@}

    /**
     * @name Internal
     * Functions that are only used internally and should be hidden from the public interface
     */
    //@{
    void updateMarkers();
    void updateClusters();
    void markClustersAsDirty();

    void getColorInfos(const int clusterIndex, QColor *fillColor, QColor *strokeColor,
                       Qt::PenStyle *strokeStyle, QString *labelText, QColor *labelColor,
                       const KMapGroupState* const overrideSelection = 0,
                       const int* const overrideCount = 0) const;

    void getColorInfos(const KMapGroupState groupState,
                       const int nMarkers,
                       QColor *fillColor, QColor *strokeColor,
                       Qt::PenStyle *strokeStyle, QString *labelText, QColor *labelColor) const;

    QString convertZoomToBackendZoom(const QString& someZoom, const QString& targetBackend) const;
    QPixmap getDecoratedPixmapForCluster(const int clusterId, const KMapGroupState* const selectedStateOverride, const int* const countOverride, QPoint* const centerPoint);
    QVariant getClusterRepresentativeMarker(const int clusterIndex, const int sortKey);
    //@}

public Q_SLOTS:

    /// @name Appearance
    //@{
    void slotZoomIn();
    void slotZoomOut();
    void slotDecreaseThumbnailSize();
    void slotIncreaseThumbnailSize();
    //@}

    /// @name Internal?
    //@{
    void slotUpdateActionsEnabled();
    void slotClustersNeedUpdating();
    void stopThumbnailTimer();
    void slotStickyModeChanged();
    //@}

Q_SIGNALS:

    void signalAltitudeLookupReady(const KMap::KMapAltitudeLookup::List& altitudes);
    void signalUngroupedModelChanged(const int index);
    void signalRegionSelectionChanged();
    void signalRemoveCurrentFilter();
    void signalStickyModeChanged();
    void signalMouseModeChanged(const KMap::MouseModes& currentMouseMode);

public:

    /** Return a string version of LibMarbleWidget release in format "major.minor.patch"
     */
    static QString MarbleWidgetVersion();

    /** Return a string version of libkmap release
     */
    static QString version();

protected:

    bool currentBackendReady() const;
    void applyCacheToBackend();
    void saveBackendToCache();
    void rebuildConfigurationMenu();
    void dropEvent(QDropEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void createActions();
    void createActionsForBackendSelection();
    void setShowPlaceholderWidget(const bool state);
    void setMapWidgetInFrame(QWidget* const widgetForFrame);
    void removeMapWidgetFromFrame();

protected Q_SLOTS:

    void slotBackendReadyChanged(const QString& backendName);
    void slotChangeBackend(QAction* action);
    void slotBackendZoomChanged(const QString& newZoom);
    void slotClustersMoved(const QIntList& clusterIndices, const QPair<int, QModelIndex>& snapTarget);
    void slotClustersClicked(const QIntList& clusterIndices);
    void slotShowThumbnailsChanged();
    void slotRequestLazyReclustering();
    void slotLazyReclusteringRequestCallBack();
    void slotItemDisplaySettingsChanged();
    void slotUngroupedModelChanged();
    void slotNewSelectionFromMap(const KMap::GeoCoordinates::Pair& sel);

    /// @name Mouse modes
    //@{
    void slotMouseModeChanged(QAction* triggeredAction);
    void slotRemoveCurrentRegionSelection();
    //@}

private:

    const QExplicitlySharedDataPointer<KMapSharedData> s;

    class KMapWidgetPrivate;
    KMapWidgetPrivate* const d;

    Q_DISABLE_COPY(KMapWidget)
};

} /* namespace KMap */

#endif /* KMAP_WIDGET_H */
