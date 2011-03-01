/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  main-window of the demo application
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
#include <kcmdlineargs.h>
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

// libkmap includes

#include "libkmap/kmap_widget.h"
#include "libkmap/itemmarkertiler.h"

// local includes

#include "dragdrophandler.h"
#include "mytreewidget.h"
#include "myimageitem.h"

using namespace KMap;

MarkerModelHelper::MarkerModelHelper(QAbstractItemModel* const itemModel, QItemSelectionModel* const itemSelectionModel)
 : ModelHelper(itemModel),
   m_itemModel(itemModel),
   m_itemSelectionModel(itemSelectionModel)
{
    connect(itemModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SIGNAL(signalModelChangedDrastically()));
}

MarkerModelHelper::~MarkerModelHelper()
{
}

QAbstractItemModel* MarkerModelHelper::model() const
{
    return m_itemModel;
}

QItemSelectionModel* MarkerModelHelper::selectionModel() const
{
    return m_itemSelectionModel;
}

bool MarkerModelHelper::itemCoordinates(const QModelIndex& index, GeoCoordinates* const coordinates) const
{
    if (!index.data(RoleCoordinates).canConvert<GeoCoordinates>())
        return false;

    if (coordinates)
        *coordinates = index.data(RoleCoordinates).value<GeoCoordinates>();

    return true;
}

void MarkerModelHelper::onIndicesMoved(const QList<QPersistentModelIndex>& movedIndices, const GeoCoordinates& targetCoordinates, const QPersistentModelIndex& targetSnapIndex)
{
    Q_UNUSED(targetSnapIndex);

    for (int i=0; i<movedIndices.count(); ++i)
    {
        m_itemModel->setData(movedIndices.at(i), QVariant::fromValue(targetCoordinates), RoleCoordinates);
    }

    emit(signalMarkersMoved(movedIndices));
}

class MyImageData
{
public:

    GeoCoordinates coordinates;
    KUrl             url;
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

    QSplitter*                          splitter;
    KMapWidget*                         mapWidget;
    MyTreeWidget*                       treeWidget;
    QPointer<QProgressBar>              progressBar;
    QList<QFuture<MyImageData> >        imageLoadingRunningFutures;
    QList<QFutureWatcher<MyImageData>*> imageLoadingFutureWatchers;
    int                                 imageLoadingTotalCount;
    int                                 imageLoadingCurrentCount;
    QList<MyImageData>                  imageLoadingBuncher;
    QTimer*                             imageLoadingBunchTimer;
    KCmdLineArgs*                       cmdLineArgs;
    KUrl                                lastImageOpenDir;

    QAbstractItemModel*                 displayMarkersModel;
    QItemSelectionModel*                selectionModel;
    MarkerModelHelper*                  markerModelHelper;
};

MainWindow::MainWindow(KCmdLineArgs* const cmdLineArgs, QWidget* const parent)
          : KMainWindow(parent), d(new MainWindowPrivate())
{
    // initialize kexiv2 before doing any multitasking
    KExiv2Iface::KExiv2::initializeExiv2();

    d->treeWidget = new MyTreeWidget(this);
    d->treeWidget->setColumnCount(2);
    d->treeWidget->setHeaderLabels(QStringList()<<i18n("Filename")<<i18n("Coordinates"));
    d->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    d->displayMarkersModel = d->treeWidget->model();
    d->selectionModel = d->treeWidget->selectionModel();
    d->markerModelHelper = new MarkerModelHelper(d->displayMarkersModel, d->selectionModel);

    ItemMarkerTiler* const mm = new ItemMarkerTiler(d->markerModelHelper, this);

    resize(512, 512);
    setWindowTitle(i18n("LibKMap demo"));
    setWindowIcon(SmallIcon( QLatin1String("applications-internet" )));
    setObjectName(QLatin1String("Demo-KMap" ));

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

    d->mapWidget = new KMapWidget(d->splitter);
    d->mapWidget->setGroupedModel(mm);
    d->mapWidget->setActive(true);
    d->mapWidget->setDragDropHandler(new DemoDragDropHandler(d->displayMarkersModel, d->mapWidget));
    d->mapWidget->setVisibleMouseModes(KMap::MouseModePan|KMap::MouseModeZoomIntoGroup|KMap::MouseModeSelectThumbnail);
    d->mapWidget->setAvailableMouseModes(KMap::MouseModePan|KMap::MouseModeZoomIntoGroup|KMap::MouseModeSelectThumbnail);

    connect(d->mapWidget, SIGNAL(signalAltitudeLookupReady(const KMap::KMapAltitudeLookup::List&)),
            this, SLOT(slotAltitudeLookupReady(const KMap::KMapAltitudeLookup::List&)));

    connect(d->markerModelHelper, SIGNAL(signalMarkersMoved(const QList<QPersistentModelIndex>&)),
            this, SLOT(slotMarkersMoved(const QList<QPersistentModelIndex>&)));

//     d->mapWidget->resize(d->mapWidget->width(), 200);
    d->splitter->addWidget(d->mapWidget);
    d->splitter->setCollapsible(0, false);
    d->splitter->setSizes(QList<int>()<<200);
    d->splitter->setStretchFactor(0, 10);

    QWidget* const dummyWidget = new QWidget(this);
    QVBoxLayout* const vbox = new QVBoxLayout(dummyWidget);

    vbox->addWidget(d->mapWidget->getControlWidget());

    vbox->addWidget(d->treeWidget);

    d->progressBar = new QProgressBar();
    d->progressBar->setFormat(i18n("Loading images - %p%"));

    d->splitter->addWidget(dummyWidget);

    readSettings();

    GeoCoordinates::List markerList;

    // ice cafe
    markerList<<GeoCoordinates::fromGeoUrl(QLatin1String("geo:51.0913031421,6.88878178596,44" ));

    // bar
    markerList<<GeoCoordinates::fromGeoUrl(QLatin1String("geo:51.06711205,6.90020261667,43" ));

    // Marienburg castle
    markerList<<GeoCoordinates::fromGeoUrl(QLatin1String("geo:51.087647318,6.88282728201,44" ));

    // head of monster
    markerList<<GeoCoordinates::fromGeoUrl(QLatin1String("geo:51.0889433167,6.88000331667,39.6" ));

    // Langenfeld
    markerList<<GeoCoordinates::fromGeoUrl(QLatin1String("geo:51.1100157609,6.94911003113,51" ));

    // Sagrada Familia in Spain
    markerList<<GeoCoordinates::fromGeoUrl(QLatin1String("geo:41.4036480511,2.1743756533,46" ));

    if (cmdLineArgs->isSet("demopoints_single")||cmdLineArgs->isSet("demopoints_group"))
    {
        for (int i=0; i<markerList.count(); ++i)
        {
            QTreeWidgetItem* const treeItem = new QTreeWidgetItem();
            treeItem->setText(0, QString::fromLatin1("item %1").arg(i));
            treeItem->setText(1, markerList.at(i).geoUrl());

            d->treeWidget->addTopLevelItem(treeItem);
        }
    }
}

MainWindow::~MainWindow()
{
    // clean up the kexiv2 memory:
    KExiv2Iface::KExiv2::cleanupExiv2();

    if (d->progressBar)
    {
        delete d->progressBar;
    }

    delete d;
}

void MainWindow::readSettings()
{
    KConfig config( QLatin1String("wmw-demo-1" ));

    const KConfigGroup groupWidgetConfig = config.group(QLatin1String("WidgetConfig"));
    d->mapWidget->readSettingsFromGroup(&groupWidgetConfig);

    KConfigGroup groupMainWindowConfig = config.group(QLatin1String("MainWindowConfig"));
    d->lastImageOpenDir                = groupMainWindowConfig.readEntry("Last Image Open Directory", KUrl());
    if (groupMainWindowConfig.hasKey("SplitterState"))
    {
        const QByteArray splitterState = QByteArray::fromBase64(groupMainWindowConfig.readEntry(QLatin1String("SplitterState"), QByteArray()));
        if (!splitterState.isEmpty())
        {
            d->splitter->restoreState(splitterState);
        }
    }
}

void MainWindow::saveSettings()
{
    KConfig config( QLatin1String("wmw-demo-1" ));

    KConfigGroup groupWidgetConfig = config.group(QLatin1String("WidgetConfig"));
    d->mapWidget->saveSettingsToGroup(&groupWidgetConfig);

    KConfigGroup groupMainWindowConfig = config.group(QLatin1String("MainWindowConfig"));
    groupMainWindowConfig.writeEntry("Last Image Open Directory", d->lastImageOpenDir);
    groupMainWindowConfig.writeEntry(QLatin1String("SplitterState"), d->splitter->saveState().toBase64());
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
        imageData.coordinates.setLatLon(lat, lon);
        imageData.coordinates.setAlt(alt);
    }

    return imageData;
}

void MainWindow::slotFutureResultsReadyAt(int startIndex, int endIndex)
{
//     kDebug()<<"future"<<startIndex<<endIndex;

    // determine the sender:
    QFutureWatcher<MyImageData>* const futureSender = reinterpret_cast<QFutureWatcher<MyImageData>*>(sender());
    KMAP_ASSERT(futureSender!=0);
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
    KMAP_ASSERT(futureIndex>=0);
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
        QTreeWidgetItem* const treeItem = new MyImageItem(currentInfo.url, currentInfo.coordinates);
        d->treeWidget->addTopLevelItem(treeItem);
    }

    d->imageLoadingBuncher.clear();
}

void MainWindow::slotMarkersMoved(const QList<QPersistentModelIndex>& markerIndices)
{
    // prepare altitude lookups
    KMapAltitudeLookup::List altitudeQueries;
    for (int i=0; i<markerIndices.count(); ++i)
    {
        const QPersistentModelIndex currentIndex = markerIndices.at(i);
        const GeoCoordinates newCoordinates = currentIndex.data(RoleCoordinates).value<GeoCoordinates>();

        KMapAltitudeLookup myLookup;
        myLookup.coordinates = newCoordinates;
        myLookup.data = QVariant::fromValue(QPersistentModelIndex(currentIndex));
        altitudeQueries << myLookup;
    }

    if (!altitudeQueries.isEmpty())
    {
        d->mapWidget->queryAltitudes(altitudeQueries, QLatin1String("geonames" ));
    }
}

void MainWindow::slotAltitudeLookupReady(const KMapAltitudeLookup::List& altitudes)
{
    for (int i=0; i<altitudes.count(); ++i)
    {
        const KMapAltitudeLookup& myLookup = altitudes.at(i);

        const QPersistentModelIndex markerIndex = myLookup.data.value<QPersistentModelIndex>();

        if (!markerIndex.isValid())
            continue;

        // TODO: why does the index return a const model???
        const QAbstractItemModel* const itemModel = markerIndex.model();
        const_cast<QAbstractItemModel*>(itemModel)->setData(markerIndex, QVariant::fromValue(myLookup.coordinates), RoleCoordinates);
    }
}

void MainWindow::slotAddImages()
{
    const KUrl::List fileNames = KFileDialog::getOpenUrls(d->lastImageOpenDir, QLatin1String("*.jpg|*.jpeg|*.png"), this, i18n("Add image files"));

    if (fileNames.isEmpty())
        return;

    d->lastImageOpenDir = fileNames.first().upUrl();

    slotScheduleImagesForLoading(fileNames);
}

void MainWindow::createMenus()
{
    QMenu* const fileMenu         = menuBar()->addMenu(i18n("File"));

    KAction* const addFilesAction = new KAction(i18n("Add images..."), fileMenu);
    fileMenu->addAction(addFilesAction);
    connect(addFilesAction, SIGNAL(triggered()),
            this, SLOT(slotAddImages()));

    menuBar()->addMenu(helpMenu());
}

KMap::ModelHelper::Flags MarkerModelHelper::modelFlags() const
{
    return FlagMovable;
}
