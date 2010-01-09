/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : main-window of the demo application
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

// Qt includes

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

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kstatusbar.h>

// LibKExiv2 includes

#include <libkexiv2/version.h>
#include <libkexiv2/kexiv2.h>

// local includes

#include "mainwindow.h"
#include "worldmapwidget2.h"

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
        imageLoadingBunchTimer(0)
    {

    }

    QSplitter* splitter;
    WorldMapWidget2* mapWidget;
    QTreeWidget* treeWidget;
    QPointer<QProgressBar> progressBar;
    QList<QFuture<MyImageData> > imageLoadingRunningFutures;
    QList<QFutureWatcher<MyImageData>*> imageLoadingFutureWatchers;
    int imageLoadingTotalCount;
    int imageLoadingCurrentCount;
    QList<MyImageData> imageLoadingBuncher;
    QTimer* imageLoadingBunchTimer;
};

MainWindow::MainWindow(QWidget* const parent)
: KMainWindow(parent), d(new MainWindowPrivate())
{
    resize(512, 512);
    setWindowTitle(i18n("WorldMapWidget2 demo"));
    setWindowIcon(SmallIcon("applications-internet"));
    setObjectName("Demo-WorldMapWidget2");

    d->imageLoadingBunchTimer = new QTimer(this);
    d->imageLoadingBunchTimer->setSingleShot(false);
    connect(d->imageLoadingBunchTimer, SIGNAL(timeout()),
            this, SLOT(slotImageLoadingBunchReady()));

    // create a status bar:
    statusBar();

    d->splitter = new QSplitter(Qt::Vertical, this);
    setCentralWidget(d->splitter);

    d->mapWidget = new WorldMapWidget2(d->splitter);
//     d->mapWidget->resize(d->mapWidget->width(), 200);
    d->splitter->addWidget(d->mapWidget);
    d->splitter->setCollapsible(0, false);
    d->splitter->setSizes(QList<int>()<<200);
    d->splitter->setStretchFactor(0, 10);

    QWidget* const dummyWidget = new QWidget(this);
    QVBoxLayout* const vbox = new QVBoxLayout(dummyWidget);

    vbox->addWidget(d->mapWidget->getControlWidget());

    d->treeWidget = new QTreeWidget(this);
    d->treeWidget->setColumnCount(2);
    d->treeWidget->setHeaderLabels(QStringList()<<i18n("Filename")<<i18n("Coordinates"));
    vbox->addWidget(d->treeWidget);

    d->progressBar = new QProgressBar();
    d->progressBar->setFormat(i18n("Loading images - %p%"));

    d->splitter->addWidget(dummyWidget);

    readSettings();

    WMWMarker::List markerList;
    for (double i = 0.0; i<6.0; ++i)
    {
        for (double j=0.0; j<6.0; ++j)
        {
            WMWMarker myMarker(WMWGeoCoordinate(52.0+i, 6.0+j));
            myMarker.setDraggable(true);
            markerList<<myMarker;
        }
    }
//     d->mapWidget->addSingleMarkers(markerList);
//     d->mapWidget->addClusterableMarkers(markerList);

    markerList.clear();
    
    // ice cafe
    markerList<<WMWMarker(WMWGeoCoordinate::fromGeoUrl("geo:51.0913031421,6.88878178596,44"));

    // bar
    markerList<<WMWMarker(WMWGeoCoordinate::fromGeoUrl("geo:51.06711205,6.90020261667,43"));

    // Marienburg castle
    markerList<<WMWMarker(WMWGeoCoordinate::fromGeoUrl("geo:51.087647318,6.88282728201,44"));

    // head of monster
    markerList<<WMWMarker(WMWGeoCoordinate::fromGeoUrl("geo:51.0889433167,6.88000331667,39.6"));

    // Langenfeld
    markerList<<WMWMarker(WMWGeoCoordinate::fromGeoUrl("geo:51.1100157609,6.94911003113,51"));

    // Sagrada Familia in Spain
    markerList<<WMWMarker(WMWGeoCoordinate::fromGeoUrl("geo:41.4036480511,2.1743756533,46"));

//     d->mapWidget->addClusterableMarkers(markerList);
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
    kDebug()<<"future"<<startIndex<<endIndex;

    // determine the sender:
    QFutureWatcher<MyImageData>* const futureSender = reinterpret_cast<QFutureWatcher<MyImageData>*>(sender());
    int futureIndex = -1;
    for (int i = 0; i<d->imageLoadingFutureWatchers.size(); ++i)
    {
        if (d->imageLoadingFutureWatchers.at(i)==futureSender)
        {
            futureIndex = i;
            break;
        }
    }
    if (futureIndex<0)
    {
        // TODO: error!
        return;
    }

    for (int index = startIndex; index<endIndex; ++index)
    {
        MyImageData newData = d->imageLoadingRunningFutures.at(futureIndex).resultAt(index);
        kDebug()<<"future"<<newData.url<<newData.coordinates.geoUrl();

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
        statusBar()->showMessage(i18n("%1 image have been loaded.", d->imageLoadingTotalCount), 3000);
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
        d->imageLoadingBunchTimer->start(30);
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

        d->mapWidget->addClusterableMarkers(WMWMarker::List()<<WMWMarker(currentInfo.coordinates));

        // add the item to the tree widget:
        QTreeWidgetItem* const treeItem = new QTreeWidgetItem();
        treeItem->setText(0, currentInfo.url.fileName());
        treeItem->setText(1, currentInfo.coordinates.geoUrl());

        d->treeWidget->addTopLevelItem(treeItem);
    }

    d->imageLoadingBuncher.clear();
}
