/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : main-window of the demo application
 *
 * Copyright (C) 2009,2010 by Michael G. Hansen <mike at mghansen dot de>
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

#include "mainwindow.moc"

// Qt includes

#include <QStandardItemModel>
#include <QCloseEvent>
#include <QComboBox>
#include <QFuture>
#include <QFutureWatcher>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSplitter>
#include <QTimer>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <qtconcurrentmap.h>

// KDE includes

#include <kaction.h>
#include <KCmdLineArgs>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmenubar.h>
#include <kstatusbar.h>

// LibKExiv2 includes

#include <libkexiv2/version.h>
#include <libkexiv2/kexiv2.h>

// local includes

#include "worldmapwidget2.h"
#include "markermodel.h"
#include "mytreewidget.h"

using namespace WMW2;

class MyImageData
{
public:
    WMWGeoCoordinate coordinates;
    KUrl url;
};

class MainWindowPrivate
{
public:
    MainWindowPrivate()
      : splitter(0),
        mapWidget(0),
        treeWidget(0),
        progressBar(0),
        imageLoadingRunningFutures(),
        imageLoadingFutureWatchers(),
        imageLoadingTotalCount(0),
        imageLoadingCurrentCount(0),
        imageLoadingBuncher(),
        imageLoadingBunchTimer(0),
        cmdLineArgs(0),
        lastImageOpenDir()
    {

    }

    QSplitter* splitter;
    WorldMapWidget2* mapWidget;
    MyTreeWidget* treeWidget;
    QPointer<QProgressBar> progressBar;
    QList<QFuture<MyImageData> > imageLoadingRunningFutures;
    QList<QFutureWatcher<MyImageData>*> imageLoadingFutureWatchers;
    int imageLoadingTotalCount;
    int imageLoadingCurrentCount;
    QList<MyImageData> imageLoadingBuncher;
    QTimer* imageLoadingBunchTimer;
    KCmdLineArgs* cmdLineArgs;
    KUrl lastImageOpenDir;

    QStandardItemModel* specialMarkersModel;
    QStandardItemModel* displayMarkersModel;
    QItemSelectionModel* selectionModel;
};

MainWindow::MainWindow(KCmdLineArgs* const cmdLineArgs, QWidget* const parent)
: KMainWindow(parent), d(new MainWindowPrivate())
{
    d->specialMarkersModel = new QStandardItemModel(this);
    d->displayMarkersModel = new QStandardItemModel(this);
    d->selectionModel = new QItemSelectionModel(d->displayMarkersModel);

    connect(d->displayMarkersModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(slotDisplayMarkersDataChanged(const QModelIndex&, const QModelIndex&)));
    connect(d->specialMarkersModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(slotSpecialMarkersDataChanged(const QModelIndex&, const QModelIndex&)));
    connect(d->selectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(slotSelectionChanged(const QItemSelection&, const QItemSelection&)));

    resize(512, 512);
    setWindowTitle(i18n("WorldMapWidget2 demo"));
    setWindowIcon(SmallIcon("applications-internet"));
    setObjectName("Demo-WorldMapWidget2");

    d->cmdLineArgs = cmdLineArgs;

    d->imageLoadingBunchTimer = new QTimer(this);
    d->imageLoadingBunchTimer->setSingleShot(false);
    connect(d->imageLoadingBunchTimer, SIGNAL(timeout()),
            this, SLOT(slotImageLoadingBunchReady()));

    // create a status bar:
    statusBar();
    createMenus();

    d->splitter = new QSplitter(Qt::Vertical, this);
    setCentralWidget(d->splitter);

    d->mapWidget = new WorldMapWidget2(d->splitter);
    d->mapWidget->setEditModeAvailable(true);
    d->mapWidget->setSpecialMarkersModel(d->specialMarkersModel, RoleCoordinates);
    d->mapWidget->setDisplayMarkersModel(d->displayMarkersModel, RoleCoordinates, d->selectionModel);

    connect(d->mapWidget, SIGNAL(signalAltitudeLookupReady(const WMW2::WMWAltitudeLookup::List&)),
            this, SLOT(slotAltitudeLookupReady(const WMW2::WMWAltitudeLookup::List&)));

    connect(d->mapWidget, SIGNAL(signalSpecialMarkersMoved(const QList<QPersistentModelIndex>&)),
            this, SLOT(slotMarkersMoved(const QList<QPersistentModelIndex>&)));

    connect(d->mapWidget, SIGNAL(signalDisplayMarkersMoved(const QList<QPersistentModelIndex>&)),
            this, SLOT(slotMarkersMoved(const QList<QPersistentModelIndex>&)));

//     d->mapWidget->resize(d->mapWidget->width(), 200);
    d->splitter->addWidget(d->mapWidget);
    d->splitter->setCollapsible(0, false);
    d->splitter->setSizes(QList<int>()<<200);
    d->splitter->setStretchFactor(0, 10);

    QWidget* const dummyWidget = new QWidget(this);
    QVBoxLayout* const vbox = new QVBoxLayout(dummyWidget);

    vbox->addWidget(d->mapWidget->getControlWidget());

    d->treeWidget = new MyTreeWidget(this);
    d->treeWidget->setColumnCount(2);
    d->treeWidget->setHeaderLabels(QStringList()<<i18n("Filename")<<i18n("Coordinates"));
    d->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    vbox->addWidget(d->treeWidget);

    connect(d->treeWidget, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotTreeWidgetSelectionChanged()));

    d->progressBar = new QProgressBar();
    d->progressBar->setFormat(i18n("Loading images - %p%"));

    d->splitter->addWidget(dummyWidget);

    readSettings();

    WMWGeoCoordinate::List markerList;

    // ice cafe
    markerList<<WMWGeoCoordinate::fromGeoUrl("geo:51.0913031421,6.88878178596,44");

    // bar
    markerList<<WMWGeoCoordinate::fromGeoUrl("geo:51.06711205,6.90020261667,43");

    // Marienburg castle
    markerList<<WMWGeoCoordinate::fromGeoUrl("geo:51.087647318,6.88282728201,44");

    // head of monster
    markerList<<WMWGeoCoordinate::fromGeoUrl("geo:51.0889433167,6.88000331667,39.6");

    // Langenfeld
    markerList<<WMWGeoCoordinate::fromGeoUrl("geo:51.1100157609,6.94911003113,51");

    // Sagrada Familia in Spain
    markerList<<WMWGeoCoordinate::fromGeoUrl("geo:41.4036480511,2.1743756533,46");

    if (cmdLineArgs->isSet("demopoints_single")||cmdLineArgs->isSet("demopoints_group"))
    {
        for (int i=0; i<markerList.count(); ++i)
        {
            QTreeWidgetItem* const treeItem = new QTreeWidgetItem();
            treeItem->setText(0, QString("item %1").arg(i));
            treeItem->setText(1, markerList.at(i).geoUrl());

            d->treeWidget->addTopLevelItem(treeItem);

            if (cmdLineArgs->isSet("demopoints_single"))
            {
                QStandardItem* const standardItem = new QStandardItem(markerList.at(i).geoUrl());
                standardItem->setData(QVariant::fromValue<WMWGeoCoordinate>(markerList.at(i)), RoleCoordinates);
                standardItem->setData(QVariant::fromValue(treeItem), RoleMyData);

                d->specialMarkersModel->appendRow(standardItem);
            }

            if (cmdLineArgs->isSet("demopoints_group"))
            {
                QStandardItem* const standardItem = new QStandardItem(markerList.at(i).geoUrl());
                standardItem->setData(QVariant::fromValue<WMWGeoCoordinate>(markerList.at(i)), RoleCoordinates);
                standardItem->setData(QVariant::fromValue(treeItem), RoleMyData);

                d->displayMarkersModel->appendRow(standardItem);
            }
        }
    }
}

MainWindow::~MainWindow()
{
    if (d->progressBar)
    {
        delete d->progressBar;
    }

    delete d;
}

void MainWindow::readSettings()
{
    KConfig config("worldmapwidget2-demo-1");

    const KConfigGroup groupWidgetConfig = config.group(QString("WidgetConfig"));
    d->mapWidget->readSettingsFromGroup(&groupWidgetConfig);

    KConfigGroup groupMainWindowConfig = config.group(QString("MainWindowConfig"));
    d->lastImageOpenDir = groupMainWindowConfig.readEntry("Last Image Open Directory", KUrl());
    if (groupMainWindowConfig.hasKey("SplitterState"))
    {
        const QByteArray splitterState = QByteArray::fromBase64(groupMainWindowConfig.readEntry(QString("SplitterState"), QByteArray()));
        if (!splitterState.isEmpty())
        {
            d->splitter->restoreState(splitterState);
        }
    }
}

void MainWindow::saveSettings()
{
    KConfig config("worldmapwidget2-demo-1");

    KConfigGroup groupWidgetConfig = config.group(QString("WidgetConfig"));
    d->mapWidget->saveSettingsToGroup(&groupWidgetConfig);

    KConfigGroup groupMainWindowConfig = config.group(QString("MainWindowConfig"));
    groupMainWindowConfig.writeEntry("Last Image Open Directory", d->lastImageOpenDir);
    groupMainWindowConfig.writeEntry(QString("SplitterState"), d->splitter->saveState().toBase64());
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
        return;

    saveSettings();
    e->accept();
}

MyImageData LoadImageData(const KUrl& urlToLoad)
{
    MyImageData imageData;
    imageData.url = urlToLoad;

    // TODO: error handling!
    KExiv2Iface::KExiv2 exiv2Iface;
    exiv2Iface.load(urlToLoad.path());
    double lat, lon, alt;
    if (exiv2Iface.getGPSInfo(alt, lat, lon))
    {
        imageData.coordinates.lat = lat;
        imageData.coordinates.lon = lon;
        imageData.coordinates.setAlt(alt);
    }

    return imageData;
}

void MainWindow::slotFutureResultsReadyAt(int startIndex, int endIndex)
{
//     kDebug()<<"future"<<startIndex<<endIndex;

    // determine the sender:
    QFutureWatcher<MyImageData>* const futureSender = reinterpret_cast<QFutureWatcher<MyImageData>*>(sender());
    WMW2_ASSERT(futureSender!=0);
    if (futureSender==0)
        return;

    int futureIndex = -1;
    for (int i = 0; i<d->imageLoadingFutureWatchers.size(); ++i)
    {
        if (d->imageLoadingFutureWatchers.at(i)==futureSender)
        {
            futureIndex = i;
            break;
        }
    }
    WMW2_ASSERT(futureIndex>=0);
    if (futureIndex<0)
    {
        // TODO: error!
        return;
    }

    for (int index = startIndex; index<endIndex; ++index)
    {
        MyImageData newData = d->imageLoadingRunningFutures.at(futureIndex).resultAt(index);
//         kDebug()<<"future"<<newData.url<<newData.coordinates.geoUrl();

        d->imageLoadingBuncher << newData;
    }

    d->imageLoadingCurrentCount+= endIndex-startIndex;
    if (d->imageLoadingCurrentCount < d->imageLoadingTotalCount)
    {
        d->progressBar->setValue(d->imageLoadingCurrentCount);
    }
    else
    {
        statusBar()->removeWidget(d->progressBar);
        statusBar()->showMessage(i18np("%1 image has been loaded.", "%1 images have been loaded.", d->imageLoadingTotalCount), 3000);
        d->imageLoadingCurrentCount = 0;
        d->imageLoadingTotalCount = 0;

        // remove the QFutures:
        qDeleteAll(d->imageLoadingFutureWatchers);
        d->imageLoadingFutureWatchers.clear();
        d->imageLoadingRunningFutures.clear();

        d->imageLoadingBunchTimer->stop();

        // force display of all images:
        QTimer::singleShot(0, this, SLOT(slotImageLoadingBunchReady()));;
    }
}

void MainWindow::slotScheduleImagesForLoading(const KUrl::List imagesToSchedule)
{
    if (imagesToSchedule.isEmpty())
        return;

    if (d->imageLoadingTotalCount==0)
    {
        statusBar()->addWidget(d->progressBar);
        d->imageLoadingBunchTimer->start(100);
    }
    d->imageLoadingTotalCount+=imagesToSchedule.count();
    d->progressBar->setRange(0, d->imageLoadingTotalCount);
    d->progressBar->setValue(d->imageLoadingCurrentCount);
    QFutureWatcher<MyImageData>* watcher = new QFutureWatcher<MyImageData>(this);

    connect(watcher, SIGNAL(resultsReadyAt(int, int)),
            this, SLOT(slotFutureResultsReadyAt(int, int)));

    QFuture<MyImageData> future = QtConcurrent::mapped(imagesToSchedule, LoadImageData);
    watcher->setFuture(future);

    d->imageLoadingRunningFutures << future;
    d->imageLoadingFutureWatchers << watcher;
}

void MainWindow::slotImageLoadingBunchReady()
{
    kDebug()<<"slotImageLoadingBunchReady";

    for (int i=0; i<d->imageLoadingBuncher.count(); ++i)
    {
        const MyImageData& currentInfo = d->imageLoadingBuncher.at(i);

        // add the item to the tree widget:
        QTreeWidgetItem* const treeItem = new QTreeWidgetItem();
        treeItem->setText(0, currentInfo.url.fileName());
        treeItem->setText(1, currentInfo.coordinates.geoUrl());

        d->treeWidget->addTopLevelItem(treeItem);

        QStandardItem* const standardItem = new QStandardItem(currentInfo.url.fileName());
        standardItem->setData(QVariant::fromValue(currentInfo.coordinates), RoleCoordinates);
        standardItem->setData(QVariant::fromValue(treeItem), RoleMyData);

        if (d->cmdLineArgs->isSet("single"))
        {
            d->specialMarkersModel->appendRow(standardItem);
        }
        else
        {
            d->displayMarkersModel->appendRow(standardItem);
            QPersistentModelIndex itemIndex = d->displayMarkersModel->indexFromItem(standardItem);
            treeItem->setData(0, RoleMyData, QVariant::fromValue(itemIndex));
        }
    }

    d->imageLoadingBuncher.clear();
}

void MainWindow::slotSpecialMarkersDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    const int startRow = topLeft.row();
    const int endRow = bottomRight.row();
    kDebug()<<startRow<<endRow;

    // only update the views at this point:
    for (int row = startRow; row<=endRow; ++row)
    {
        const QModelIndex currentIndex = d->specialMarkersModel->index(row, 0, topLeft.parent());
        const WMWGeoCoordinate newCoordinates = d->specialMarkersModel->data(currentIndex, RoleCoordinates).value<WMWGeoCoordinate>();
        QTreeWidgetItem* const treeItem = d->specialMarkersModel->data(currentIndex, RoleMyData).value<QTreeWidgetItem*>();
        if (!treeItem)
            continue;

        treeItem->setText(1, newCoordinates.geoUrl());
    }
}

void MainWindow::slotDisplayMarkersDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    const int startRow = topLeft.row();
    const int endRow = bottomRight.row();

    // only update the views at this point:
    for (int row = startRow; row<=endRow; ++row)
    {
        const QModelIndex currentIndex = d->displayMarkersModel->index(row, 0, topLeft.parent());
        const WMWGeoCoordinate newCoordinates = d->displayMarkersModel->data(currentIndex, RoleCoordinates).value<WMWGeoCoordinate>();
        QTreeWidgetItem* const treeItem = d->displayMarkersModel->data(currentIndex, RoleMyData).value<QTreeWidgetItem*>();
        if (!treeItem)
            continue;

        treeItem->setText(1, newCoordinates.geoUrl());
    }
}

void MainWindow::slotMarkersMoved(const QList<QPersistentModelIndex>& markerIndices)
{
    // prepare altitude lookups
    WMWAltitudeLookup::List altitudeQueries;
    for (int i=0; i<markerIndices.count(); ++i)
    {
        const QPersistentModelIndex currentIndex = markerIndices.at(i);
        const WMWGeoCoordinate newCoordinates = currentIndex.data(RoleCoordinates).value<WMWGeoCoordinate>();

        WMWAltitudeLookup myLookup;
        myLookup.coordinates = newCoordinates;
        myLookup.data = QVariant::fromValue(QPersistentModelIndex(currentIndex));
        altitudeQueries << myLookup;
    }

    if (!altitudeQueries.isEmpty())
    {
        d->mapWidget->queryAltitudes(altitudeQueries, "geonames");
    }
}

void MainWindow::slotAltitudeLookupReady(const WMWAltitudeLookup::List& altitudes)
{
    for (int i=0; i<altitudes.count(); ++i)
    {
        const WMWAltitudeLookup& myLookup = altitudes.at(i);

        const QPersistentModelIndex markerIndex = myLookup.data.value<QPersistentModelIndex>();

        if (!markerIndex.isValid())
            continue;

        // TODO: why does the index return a const model???
        const QAbstractItemModel* const itemModel = markerIndex.model();
        const_cast<QAbstractItemModel*>(itemModel)->setData(markerIndex, QVariant::fromValue(myLookup.coordinates), RoleCoordinates);
    }
}

void MainWindow::slotTreeWidgetSelectionChanged()
{
    for (int i=0; i<d->treeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* const treeItem = d->treeWidget->topLevelItem(i);
        const QPersistentModelIndex itemIndex = treeItem->data(0, RoleMyData).value<QPersistentModelIndex>();
        if (!itemIndex.isValid())
            continue;

        d->selectionModel->select(itemIndex, treeItem->isSelected() ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
    }
}

void MainWindow::slotAddImages()
{
    const KUrl::List fileNames = KFileDialog::getOpenUrls(d->lastImageOpenDir, QString("*.jpg|*.jpeg|*.png"), this, i18n("Add image files"));

    if (fileNames.isEmpty())
        return;

    d->lastImageOpenDir = fileNames.first().upUrl();

    slotScheduleImagesForLoading(fileNames);
}

void MainWindow::createMenus()
{
    QMenu* const fileMenu = menuBar()->addMenu(i18n("File"));

    KAction* const addFilesAction = new KAction(i18n("Add images..."), fileMenu);
    fileMenu->addAction(addFilesAction);
    connect(addFilesAction, SIGNAL(triggered()),
            this, SLOT(slotAddImages()));

    menuBar()->addMenu(helpMenu());
}

void MainWindow::slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    kDebug()<<selected<<deselected;

    for (int i=0; i<selected.count(); ++i)
    {
        const QItemSelectionRange selectionRange = selected.at(i);
        for (int row = selectionRange.top(); row<=selectionRange.bottom(); ++row)
        {
            const QModelIndex currentIndex = d->displayMarkersModel->index(row, 0, selectionRange.parent());

            QTreeWidgetItem* const treeItem = d->displayMarkersModel->data(currentIndex, RoleMyData).value<QTreeWidgetItem*>();
            WMW2_ASSERT(treeItem!=0);
            if (!treeItem)
                continue;

            if (!treeItem->isSelected())
            {
                treeItem->setSelected(true);
            }
        }
    }

    for (int i=0; i<deselected.count(); ++i)
    {
        const QItemSelectionRange selectionRange = deselected.at(i);
        for (int row = selectionRange.top(); row<=selectionRange.bottom(); ++row)
        {
            const QModelIndex currentIndex = d->displayMarkersModel->index(row, 0, selectionRange.parent());

            QTreeWidgetItem* const treeItem = d->displayMarkersModel->data(currentIndex, RoleMyData).value<QTreeWidgetItem*>();
            WMW2_ASSERT(treeItem!=0);
            if (!treeItem)
                continue;

            if (treeItem->isSelected())
            {
                treeItem->setSelected(false);
            }
        }
    }
}
