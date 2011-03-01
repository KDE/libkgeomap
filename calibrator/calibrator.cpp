/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-09-18
 * @brief  A tool to calibrate the tiling levels used in libkmap
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
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

#include "calibrator.moc"

// Qt includes

#include <QAction>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <KAboutData>
#include <KCmdLineArgs>

// local includes

#include "libkmap/abstractmarkertiler.h"
#include "libkmap/itemmarkertiler.h"
#include "libkmap/kmap_widget.h"
#include "libkmap/version.h"

const int CoordinatesRole = Qt::UserRole + 1;

class CalibratorModelHelper::CalibratorModelHelperPrivate
{
public:
    CalibratorModelHelperPrivate()
    {
    }

    QStandardItemModel* model;
};

CalibratorModelHelper::CalibratorModelHelper(QStandardItemModel* const model, QObject* const parent)
 : ModelHelper(parent), d(new CalibratorModelHelperPrivate())
{
    d->model = model;
}

CalibratorModelHelper::~CalibratorModelHelper()
{
    delete d;
}

QAbstractItemModel* CalibratorModelHelper::model() const
{
    return d->model;
}

QItemSelectionModel* CalibratorModelHelper::selectionModel() const
{
    return 0;
}

bool CalibratorModelHelper::itemCoordinates(const QModelIndex& index, KMap::GeoCoordinates* const coordinates) const
{
    if (!index.isValid())
        return false;

    const QVariant coordinatesVariant = index.data(CoordinatesRole);
    KMap::GeoCoordinates itemCoordinates = coordinatesVariant.value<KMap::GeoCoordinates>();

    if (coordinates)
        *coordinates = itemCoordinates;

    return itemCoordinates.hasCoordinates();
}

void CalibratorModelHelper::setItemCoordinates(const QModelIndex& index, const KMap::GeoCoordinates& coordinates)
{
    if (!index.isValid())
        return;

    d->model->setData(index, QVariant::fromValue(coordinates), CoordinatesRole);
}

KMap::ModelHelper::Flags CalibratorModelHelper::modelFlags() const
{
    return FlagVisible;
}

class Calibrator::CalibratorPrivate
{
public:
    CalibratorPrivate()
    {
    }

    QHBoxLayout                *hBoxLayout;
    QList<QPair<QWidget*, KMap::KMapWidget*> >        extraWidgetHolders;
    QStandardItemModel         *model;
    CalibratorModelHelper      *modelHelper;
    KMap::ItemMarkerTiler      *markerTiler;

    QButtonGroup               *groupingMode;
    QSpinBox                   *sbLevel;
    QLineEdit                  *zoomDisplay;

    QTimer                     *zoomDisplayTimer;
};

Calibrator::Calibrator()
 : KMainWindow(), d(new CalibratorPrivate())
{
    d->model = new QStandardItemModel(this);
    d->modelHelper = new CalibratorModelHelper(d->model, this);
    d->markerTiler = new KMap::ItemMarkerTiler(d->modelHelper, this);

    QVBoxLayout* const vboxLayout1 = new QVBoxLayout();
    QWidget* const dummy1 = new QWidget(this);
    dummy1->setLayout(vboxLayout1);
    setCentralWidget(dummy1);

    d->hBoxLayout = new QHBoxLayout();
    vboxLayout1->addLayout(d->hBoxLayout);

    d->groupingMode = new QButtonGroup(this);
    d->groupingMode->setExclusive(true);
    QRadioButton* const buttonGrouped = new QRadioButton(i18n("Grouped"), this);
    d->groupingMode->addButton(buttonGrouped, 0);
    QRadioButton* const buttonUngrouped = new QRadioButton(i18n("Ungrouped"), this);
    d->groupingMode->addButton(buttonUngrouped, 1);
    buttonGrouped->setChecked(true);

    d->sbLevel = new QSpinBox(this);
    d->sbLevel->setMinimum(1);
    d->sbLevel->setMaximum(KMap::TileIndex::MaxLevel);
    QLabel* const labelsbLevel = new QLabel(i18nc("Tile level", "Level:"), this);
    labelsbLevel->setBuddy(d->sbLevel);

    d->zoomDisplay = new QLineEdit(this);
    d->zoomDisplay->setReadOnly(true);
    QLabel* const labelZoomDisplay = new QLabel(i18n("Zoom:"), this);
    labelZoomDisplay->setBuddy(d->zoomDisplay);

    QHBoxLayout* const hboxLayout1 = new QHBoxLayout(this);
    hboxLayout1->addWidget(new QLabel(i18n("Display mode:"), this));
    hboxLayout1->addWidget(buttonGrouped);
    hboxLayout1->addWidget(buttonUngrouped);
    hboxLayout1->addWidget(labelsbLevel);
    hboxLayout1->addWidget(d->sbLevel);
    hboxLayout1->addWidget(labelZoomDisplay);
    hboxLayout1->addWidget(d->zoomDisplay);
    hboxLayout1->addStretch(10);
    vboxLayout1->addLayout(hboxLayout1);

    QHBoxLayout* const hboxLayout2 = new QHBoxLayout(this);
    QPushButton* const pbAddMap = new QPushButton(i18n("Add Map Widget"), this);
    hboxLayout2->addWidget(pbAddMap);
    QPushButton* const pbRemoveMap = new QPushButton(i18n("Remove Map Widget"), this);
    hboxLayout2->addWidget(pbRemoveMap);
    vboxLayout1->addLayout(hboxLayout2);

    connect(d->groupingMode, SIGNAL(buttonClicked(int)),
            this, SLOT(updateGroupingMode()));

    connect(d->sbLevel, SIGNAL(valueChanged(int)),
            this, SLOT(updateMarkers()));

    connect(pbAddMap, SIGNAL(clicked()),
            this, SLOT(slotAddMapWidget()));

    connect(pbRemoveMap, SIGNAL(clicked()),
            this, SLOT(slotRemoveMapWidget()));

    updateMarkers();
    updateGroupingMode();

    slotAddMapWidget();

    d->zoomDisplayTimer = new QTimer(this);
    d->zoomDisplayTimer->start(200);

    connect(d->zoomDisplayTimer, SIGNAL(timeout()),
            this, SLOT(updateZoomView()));
}

Calibrator::~Calibrator()
{
    delete d;
}

void Calibrator::updateGroupingMode()
{
    const bool shouldBeGrouped = d->groupingMode->checkedId()==0;

    for (int i = 0; i<d->extraWidgetHolders.count(); ++i)
    {
        KMap::KMapWidget* const mapWidget = d->extraWidgetHolders.at(i).second;

        if (shouldBeGrouped)
        {
            mapWidget->removeUngroupedModel(d->modelHelper);
            mapWidget->setGroupedModel(d->markerTiler);
        }
        else
        {
            mapWidget->setGroupedModel(0);
            mapWidget->addUngroupedModel(d->modelHelper);
        }
    }
}

void Calibrator::addMarkerAt(const KMap::GeoCoordinates& coordinates)
{
    kDebug()<<coordinates;
    QStandardItem* const item = new QStandardItem(coordinates.geoUrl());
    item->setData(QVariant::fromValue(coordinates), CoordinatesRole);

    d->model->appendRow(item);
}

void Calibrator::updateMarkers()
{
    d->model->clear();

    const int newLevel = d->sbLevel->value();
    const int Tiling = KMap::TileIndex::Tiling;

    // add markers in all four corners and in the middle of the edges:
    typedef QPair<int, int> QIntPair;
    QList<QIntPair> partialTilePositions;

    // corners:
    partialTilePositions
        << QIntPair(0, 0)
        << QIntPair(Tiling-1, Tiling-1)
        << QIntPair(Tiling*(Tiling-1), Tiling*(Tiling-1))
        << QIntPair(Tiling*Tiling-1, Tiling*Tiling-1);

    // middle of edges:
    partialTilePositions
        << QIntPair(Tiling/2, 0)
        << QIntPair(Tiling*(Tiling/2), 0)
        << QIntPair(Tiling*(Tiling/2)+Tiling-1, (Tiling-1))
        << QIntPair(Tiling*Tiling-Tiling/2-1, Tiling*Tiling-1);

    // center of the map:
    partialTilePositions
        << QIntPair(Tiling*(Tiling/2)+Tiling/2, 0)
        << QIntPair(Tiling*(Tiling/2-1)+Tiling/2, Tiling*(Tiling-1))
        << QIntPair(Tiling*(Tiling/2-1)+Tiling/2-1, Tiling*Tiling-1)
        << QIntPair(Tiling*(Tiling/2)+Tiling/2-1, Tiling-1);

    // at +/- ~70 degrees (cutoff of Mercator projection is at 80):
    partialTilePositions
        << QIntPair(Tiling, 0)
        << QIntPair(2*Tiling-1, Tiling-1)
        << QIntPair(Tiling*(Tiling-2), Tiling*(Tiling-1))
        << QIntPair(Tiling*(Tiling-1)-1, Tiling*Tiling-1);

    for (int ptp = 0; ptp<partialTilePositions.count(); ++ptp)
    {
        QIntPair currentPair = partialTilePositions.at(ptp);
        const int level0Index = currentPair.first;
        const int followingIndex = currentPair.second;

        KMap::TileIndex markerIndex;
        markerIndex.appendLinearIndex(level0Index);

        for (int level = 1; level < newLevel-2; level++)
        {
            markerIndex.appendLinearIndex(followingIndex);
        }

        const int smallPart = followingIndex % Tiling;
        for (int i = -1; i<=1; ++i)
        {
            if ((smallPart+i>=0)&&(smallPart+i<Tiling))
            {
                for (int j = -1; j<=1; ++j)
                {
                    const int newLinIndex = followingIndex + i + j*Tiling;
                    if ((newLinIndex>=0)&&(newLinIndex<Tiling*Tiling))
                    {
                        KMap::TileIndex newIndex = markerIndex;
                        newIndex.appendLinearIndex(newLinIndex);
                        addMarkerAt(newIndex.toCoordinates());
//                         for (int corner = 1; corner<=4; ++corner)
//                         {
//                             addMarkerAt(newIndex.toCoordinates(KMap::TileIndex::CornerPosition(corner)));
//                         }
                    }
                }
            }
        }
    }

    kDebug()<<d->model->rowCount();
}

void Calibrator::updateZoomView()
{
    if (d->extraWidgetHolders.isEmpty())
    {
        return;
    }

    KMap::KMapWidget* const firstMapWidget = d->extraWidgetHolders.first().second;
    const QString newZoom = firstMapWidget->getZoom();
    if (newZoom!=d->zoomDisplay->text())
    {
        d->zoomDisplay->setText(newZoom);
    }
}

void Calibrator::slotAddMapWidget()
{
    QVBoxLayout* boxLayout = new QVBoxLayout();
    KMap::KMapWidget* mapWidget = new KMap::KMapWidget();
    boxLayout->addWidget(mapWidget);
    boxLayout->addWidget(mapWidget->getControlWidget());

    QAction* const activateMapAction = new QAction(i18nc("Set the widget active", "Active"), mapWidget);
    activateMapAction->setData(QVariant::fromValue<void*>(mapWidget));
    activateMapAction->setCheckable(true);
    QToolButton* const toolButton = new QToolButton(mapWidget);
    toolButton->setDefaultAction(activateMapAction);
    mapWidget->addWidgetToControlWidget(toolButton);

    connect(activateMapAction, SIGNAL(triggered(bool)),
            this, SLOT(slotActivateMapActionTriggered(bool)));

    QWidget* const dummyWidget = new QWidget();
    dummyWidget->setLayout(boxLayout);
    d->extraWidgetHolders.append(QPair<QWidget*, KMap::KMapWidget*>(dummyWidget, mapWidget));

    d->hBoxLayout->addWidget(dummyWidget);

    updateGroupingMode();
}

void Calibrator::slotRemoveMapWidget()
{
    if (d->extraWidgetHolders.isEmpty())
    {
        return;
    }

    QPair<QWidget*, KMap::KMapWidget*> info =  d->extraWidgetHolders.takeLast();
    d->hBoxLayout->removeWidget(info.first);
    delete info.first;
}

void Calibrator::slotActivateMapActionTriggered(bool state)
{
    QAction* const senderAction = qobject_cast<QAction*>(sender());
    if (!senderAction)
    {
        return;
    }

    KMap::KMapWidget* const mapWidget = static_cast<KMap::KMapWidget*>(senderAction->data().value<void*>());
    mapWidget->setActive(state);
}

int main(int argc, char* argv[])
{
    KAboutData aboutData(
        "calibrator-kmap",
        0,
        ki18n("KMap calibration tool"),
        kmap_version,                                      // version
        ki18n("Used to calibrate the KMap library tiling level"),
        KAboutData::License_GPL,
        ki18n("(c) 2010 Michael G. Hansen"),
        ki18n(""),                                         // optional text
        "http://www.digikam.org/sharedlibs",               // URI of homepage
        ""                                                 // bugs e-mail address
    );

    aboutData.addAuthor(ki18n("Michael G. Hansen"),
                        ki18n("KMap library"),
                        "mike@mghansen.de",
                        "http://www.mghansen.de");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app;

    Calibrator* calibrator = new Calibrator();
    calibrator->show();

    return app.exec();
}

