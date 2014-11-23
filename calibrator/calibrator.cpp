/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-09-18
 * @brief  A tool to calibrate the tiling levels used in libkgeomap
 *
 * @author Copyright (C) 2009-2010,2014 by Michael G. Hansen
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

#include "calibrator.h"

// Qt includes

#include <QAction>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

// KDE includes


#include <KAboutData>

#include <klineedit.h>
#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>

// local includes

#include "abstractmarkertiler.h"
#include "itemmarkertiler.h"
#include "kgeomap_widget.h"
#include "geocoordinates.h"
//#include "version.h"

const int CoordinatesRole = Qt::UserRole + 1;

class CalibratorModelHelper::Private
{
public:

    Private()
     : model(0)
    {
    }

    QStandardItemModel* model;
};

CalibratorModelHelper::CalibratorModelHelper(QStandardItemModel* const model, QObject* const parent)
    : ModelHelper(parent), d(new Private())
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

bool CalibratorModelHelper::itemCoordinates(const QModelIndex& index, KGeoMap::GeoCoordinates* const coordinates) const
{
    if (!index.isValid())
        return false;

    const QVariant coordinatesVariant       = index.data(CoordinatesRole);
    KGeoMap::GeoCoordinates itemCoordinates = coordinatesVariant.value<KGeoMap::GeoCoordinates>();

    if (coordinates)
        *coordinates = itemCoordinates;

    return itemCoordinates.hasCoordinates();
}

void CalibratorModelHelper::setItemCoordinates(const QModelIndex& index, const KGeoMap::GeoCoordinates& coordinates)
{
    if (!index.isValid())
        return;

    d->model->setData(index, QVariant::fromValue(coordinates), CoordinatesRole);
}

KGeoMap::ModelHelper::Flags CalibratorModelHelper::modelFlags() const
{
    return FlagVisible;
}

// ---------------------------------------------------------------------------------------------------------------------

class Calibrator::Private
{
public:
    Private()
     : hBoxLayout(0),
       model(0),
       modelHelper(0),
       markerTiler(0),
       groupingMode(0),
       sbLevel(0),
       zoomDisplay(0),
       zoomDisplayTimer(0)
    {
    }

    QHBoxLayout*                                     hBoxLayout;
    QList<QPair<QWidget*, KGeoMap::KGeoMapWidget*> > extraWidgetHolders;
    QStandardItemModel*                              model;
    CalibratorModelHelper*                           modelHelper;
    KGeoMap::ItemMarkerTiler*                        markerTiler;

    QButtonGroup*                                    groupingMode;
    QSpinBox*                                        sbLevel;
    KLineEdit*                                       zoomDisplay;

    QTimer*                                          zoomDisplayTimer;
};

Calibrator::Calibrator()
    : KMainWindow(), d(new Private())
{
    d->model       = new QStandardItemModel(this);
    d->modelHelper = new CalibratorModelHelper(d->model, this);
    d->markerTiler = new KGeoMap::ItemMarkerTiler(d->modelHelper, this);

    QVBoxLayout* const vboxLayout1 = new QVBoxLayout();
    QWidget* const dummy1          = new QWidget(this);
    dummy1->setLayout(vboxLayout1);
    setCentralWidget(dummy1);

    d->hBoxLayout = new QHBoxLayout();
    vboxLayout1->addLayout(d->hBoxLayout);

    d->groupingMode                     = new QButtonGroup(this);
    d->groupingMode->setExclusive(true);
    QRadioButton* const buttonGrouped   = new QRadioButton(i18n("Grouped"), this);
    d->groupingMode->addButton(buttonGrouped, 0);
    QRadioButton* const buttonUngrouped = new QRadioButton(i18n("Ungrouped"), this);
    d->groupingMode->addButton(buttonUngrouped, 1);
    buttonGrouped->setChecked(true);

    d->sbLevel                 = new QSpinBox(this);
    d->sbLevel->setMinimum(1);
    d->sbLevel->setMaximum(KGeoMap::TileIndex::MaxLevel);
    QLabel* const labelsbLevel = new QLabel(i18nc("Tile level", "Level:"), this);
    labelsbLevel->setBuddy(d->sbLevel);

    d->zoomDisplay                 = new KLineEdit(this);
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
    QPushButton* const pbAddMap    = new QPushButton(i18n("Add Map Widget"), this);
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

    for (int i = 0; i < d->extraWidgetHolders.count(); ++i)
    {
        KGeoMap::KGeoMapWidget* const mapWidget = d->extraWidgetHolders.at(i).second;

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

void Calibrator::addMarkerAt(const KGeoMap::GeoCoordinates& coordinates)
{
    kDebug() << coordinates;
    QStandardItem* const item = new QStandardItem(coordinates.geoUrl());
    item->setData(QVariant::fromValue(coordinates), CoordinatesRole);

    d->model->appendRow(item);
}

void Calibrator::updateMarkers()
{
    d->model->clear();

    const int newLevel = d->sbLevel->value();
    const int Tiling   = KGeoMap::TileIndex::Tiling;

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
        QIntPair currentPair     = partialTilePositions.at(ptp);
        const int level0Index    = currentPair.first;
        const int followingIndex = currentPair.second;

        KGeoMap::TileIndex markerIndex;
        markerIndex.appendLinearIndex(level0Index);

        for (int level = 1; level < newLevel-2; level++)
        {
            markerIndex.appendLinearIndex(followingIndex);
        }

        const int smallPart = followingIndex % Tiling;

        for (int i = -1; i <= 1; ++i)
        {
            if ((smallPart+i >= 0) && (smallPart+i < Tiling))
            {
                for (int j = -1; j<=1; ++j)
                {
                    const int newLinIndex = followingIndex + i + j*Tiling;

                    if ((newLinIndex >= 0) && (newLinIndex < Tiling*Tiling))
                    {
                        KGeoMap::TileIndex newIndex = markerIndex;
                        newIndex.appendLinearIndex(newLinIndex);
                        addMarkerAt(newIndex.toCoordinates());
//                         for (int corner = 1; corner<=4; ++corner)
//                         {
//                             addMarkerAt(newIndex.toCoordinates(KGeoMap::TileIndex::CornerPosition(corner)));
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

    KGeoMap::KGeoMapWidget* const firstMapWidget = d->extraWidgetHolders.first().second;
    const QString newZoom                        = firstMapWidget->getZoom();

    if (newZoom!=d->zoomDisplay->text())
    {
        d->zoomDisplay->setText(newZoom);
    }
}

void Calibrator::slotAddMapWidget()
{
    QVBoxLayout* const boxLayout            = new QVBoxLayout();
    KGeoMap::KGeoMapWidget* const mapWidget = new KGeoMap::KGeoMapWidget();
    boxLayout->addWidget(mapWidget);
    boxLayout->addWidget(mapWidget->getControlWidget());

    QAction* const activateMapAction = new QAction(i18nc("Set the widget active", "Active"), mapWidget);
    activateMapAction->setData(QVariant::fromValue<void*>(mapWidget));
    activateMapAction->setCheckable(true);
    QToolButton* const toolButton    = new QToolButton(mapWidget);
    toolButton->setDefaultAction(activateMapAction);
    mapWidget->addWidgetToControlWidget(toolButton);

    connect(activateMapAction, SIGNAL(triggered(bool)),
            this, SLOT(slotActivateMapActionTriggered(bool)));

    QWidget* const dummyWidget = new QWidget();
    dummyWidget->setLayout(boxLayout);
    d->extraWidgetHolders.append(QPair<QWidget*, KGeoMap::KGeoMapWidget*>(dummyWidget, mapWidget));

    d->hBoxLayout->addWidget(dummyWidget);

    updateGroupingMode();
}

void Calibrator::slotRemoveMapWidget()
{
    if (d->extraWidgetHolders.isEmpty())
    {
        return;
    }

    QPair<QWidget*, KGeoMap::KGeoMapWidget*> info = d->extraWidgetHolders.takeLast();
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

    KGeoMap::KGeoMapWidget* const mapWidget = static_cast<KGeoMap::KGeoMapWidget*>(senderAction->data().value<void*>());
    mapWidget->setActive(state);
}

int main(int argc, char* argv[])
{
    KAboutData aboutData("calibrator-kgeomap", i18n("KGeoMap calibration tool"),  "kgeomap_version"); // TODO fix version
    aboutData.setShortDescription(i18n("Used to calibrate the KGeoMap library tiling level"));
    aboutData.setLicense(KAboutLicense::GPL);
    aboutData.setCopyrightStatement(i18n("(c) 2010 Michael G. Hansen"));
    aboutData.setHomepage("http://www.digikam.org/sharedlibs");

    aboutData.addAuthor(i18n("Michael G. Hansen"),
                        i18n("KGeoMap library"),
                        "mike@mghansen.de",
                        "http://www.mghansen.de");

    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    //PORTING SCRIPT: adapt aboutdata variable if necessary
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);


    Calibrator* const calibrator = new Calibrator();
    calibrator->show();

    return app.exec();
}
