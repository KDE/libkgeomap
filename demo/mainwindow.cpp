/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  main-window of the demo application
 *
 * @author Copyright (C) 2009-2010, 2014 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2014 by Justus Schwartz
 *         <a href="mailto:justus at gmx dot li">justus at gmx dot li</a>
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

#include "mainwindow.h"

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
#include <QtConcurrentMap>

// KDE includes

#include <QAction>
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

#include <KExiv2/KExiv2>

// libkgeomap includes

#include "src/lookup_altitude.h"
#include "src/lookup_factory.h"
#include "src/kgeomap_widget.h"
#include "src/itemmarkertiler.h"

// local includes

#include "dragdrophandler.h"
#include "mytreewidget.h"
#include "myimageitem.h"

using namespace KGeoMap;

MarkerModelHelper::MarkerModelHelper(QAbstractItemModel* const itemModel, QItemSelectionModel* const itemSelectionModel)
    : ModelHelper(itemModel),
      m_itemModel(itemModel),
      m_itemSelectionModel(itemSelectionModel)
{
    connect(itemModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
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

    for (int i = 0; i < movedIndices.count(); ++i)
    {
        m_itemModel->setData(movedIndices.at(i), QVariant::fromValue(targetCoordinates), RoleCoordinates);
    }

    emit(signalMarkersMoved(movedIndices));
}

// ----------------------------------------------------------------------

class MyImageData
{
public:

    GeoCoordinates coordinates;
    QUrl           url;
};

// ----------------------------------------------------------------------

MyTrackModelHelper::MyTrackModelHelper(QAbstractItemModel* const imageItemsModel)
    : QObject(imageItemsModel),
      m_itemModel(imageItemsModel)
{
    connect(imageItemsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotTrackModelChanged()));

    connect(imageItemsModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotTrackModelChanged()));

    connect(imageItemsModel, SIGNAL(modelReset()),
            this, SLOT(slotTrackModelChanged()));
}

void MyTrackModelHelper::slotTrackModelChanged()
{
    m_tracks.clear();

    TrackManager::Track track;

    for (int row = 0; row < m_itemModel->rowCount(); ++row)
    {
        const QModelIndex currentIndex = m_itemModel->index(row, 0);

        if (!currentIndex.data(RoleCoordinates).canConvert<GeoCoordinates>())
            continue;

        const GeoCoordinates markerCoordinates = currentIndex.data(RoleCoordinates).value<GeoCoordinates>();
        TrackManager::TrackPoint trackPoint;
        trackPoint.coordinates                 = markerCoordinates;
        track.points << trackPoint;
    }

    m_tracks << track;

    emit signalModelChanged();
}

TrackManager::Track::List MyTrackModelHelper::getTracks() const
{
    return m_tracks;
}

// ----------------------------------------------------------------------

class MainWindow::Private
{
public:

    Private()
      : splitter(0),
        mapWidget(0),
        lookupAltitudeList(),
        treeWidget(0),
        progressBar(0),
        imageLoadingRunningFutures(),
        imageLoadingFutureWatchers(),
        imageLoadingTotalCount(0),
        imageLoadingCurrentCount(0),
        imageLoadingBuncher(),
        imageLoadingBunchTimer(0),
        cmdLineArgs(0),
        lastImageOpenDir(),
        displayMarkersModel(0),
        selectionModel(0),
        markerModelHelper(0),
        trackModelHelper(0)
    {
    }

    QSplitter*                          splitter;
    KGeoMapWidget*                      mapWidget;
    QList<LookupAltitude*>              lookupAltitudeList;
    MyTreeWidget*                       treeWidget;
    QPointer<QProgressBar>              progressBar;
    QList<QFuture<MyImageData> >        imageLoadingRunningFutures;
    QList<QFutureWatcher<MyImageData>*> imageLoadingFutureWatchers;
    int                                 imageLoadingTotalCount;
    int                                 imageLoadingCurrentCount;
    QList<MyImageData>                  imageLoadingBuncher;
    QTimer*                             imageLoadingBunchTimer;
    KCmdLineArgs*                       cmdLineArgs;
    QUrl                                lastImageOpenDir;

    QAbstractItemModel*                 displayMarkersModel;
    QItemSelectionModel*                selectionModel;
    MarkerModelHelper*                  markerModelHelper;
    MyTrackModelHelper*                 trackModelHelper;
};

MainWindow::MainWindow(KCmdLineArgs* const cmdLineArgs, QWidget* const parent)
    : KMainWindow(parent), d(new Private())
{
    // initialize kexiv2 before doing any multitasking
    KExiv2Iface::KExiv2::initializeExiv2();

    d->treeWidget = new MyTreeWidget(this);
    d->treeWidget->setColumnCount(2);
    d->treeWidget->setHeaderLabels(QStringList()<<i18n("Filename")<<i18n("Coordinates"));
    d->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    d->displayMarkersModel    = d->treeWidget->model();
    d->selectionModel         = d->treeWidget->selectionModel();
    d->markerModelHelper      = new MarkerModelHelper(d->displayMarkersModel, d->selectionModel);
    d->trackModelHelper       = new MyTrackModelHelper(d->treeWidget->model());
    ItemMarkerTiler* const mm = new ItemMarkerTiler(d->markerModelHelper, this);

    resize(512, 512);
    setWindowTitle(i18n("LibKGeoMap demo"));
    setWindowIcon(SmallIcon( QLatin1String("applications-internet" )));
    setObjectName(QLatin1String("Demo-KGeoMap" ));

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

    d->mapWidget = new KGeoMapWidget(d->splitter);
    d->mapWidget->setGroupedModel(mm);
    d->mapWidget->setActive(true);
    d->mapWidget->setDragDropHandler(new DemoDragDropHandler(d->displayMarkersModel, d->mapWidget));
    d->mapWidget->setVisibleMouseModes(KGeoMap::MouseModePan|KGeoMap::MouseModeZoomIntoGroup|KGeoMap::MouseModeSelectThumbnail);
    d->mapWidget->setAvailableMouseModes(KGeoMap::MouseModePan|KGeoMap::MouseModeZoomIntoGroup|KGeoMap::MouseModeSelectThumbnail);
//     d->mapWidget->setTrackModel(d->trackModelHelper);

    connect(d->markerModelHelper, SIGNAL(signalMarkersMoved(QList<QPersistentModelIndex>)),
            this, SLOT(slotMarkersMoved(QList<QPersistentModelIndex>)));

//     d->mapWidget->resize(d->mapWidget->width(), 200);
    d->splitter->addWidget(d->mapWidget);
    d->splitter->setCollapsible(0, false);
    d->splitter->setSizes(QList<int>()<<200);
    d->splitter->setStretchFactor(0, 10);

    QWidget* const dummyWidget = new QWidget(this);
    QVBoxLayout* const vbox    = new QVBoxLayout(dummyWidget);

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

    if (cmdLineArgs->isSet("demopoints_single") || cmdLineArgs->isSet("demopoints_group"))
    {
        for (int i = 0; i < markerList.count(); ++i)
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
    d->lastImageOpenDir                = groupMainWindowConfig.readEntry("Last Image Open Directory", QUrl());

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
    groupMainWindowConfig.writeEntry("Last Image Open Directory", d->lastImageOpenDir.toLocalFile());
    groupMainWindowConfig.writeEntry(QLatin1String("SplitterState"), d->splitter->saveState().toBase64());
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
        return;

    saveSettings();
    e->accept();
}

MyImageData LoadImageData(const QUrl& urlToLoad)
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
    KGEOMAP_ASSERT(futureSender!=0);

    if (futureSender == 0)
        return;

    int futureIndex = -1;

    for (int i = 0; i < d->imageLoadingFutureWatchers.size(); ++i)
    {
        if (d->imageLoadingFutureWatchers.at(i) == futureSender)
        {
            futureIndex = i;
            break;
        }
    }

    KGEOMAP_ASSERT(futureIndex >= 0);

    if (futureIndex < 0)
    {
        // TODO: error!
        return;
    }

    for (int index = startIndex; index < endIndex; ++index)
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
        d->imageLoadingTotalCount   = 0;

        // remove the QFutures:
        qDeleteAll(d->imageLoadingFutureWatchers);
        d->imageLoadingFutureWatchers.clear();
        d->imageLoadingRunningFutures.clear();

        d->imageLoadingBunchTimer->stop();

        // force display of all images:
        QTimer::singleShot(0, this, SLOT(slotImageLoadingBunchReady()));;
    }
}

void MainWindow::slotScheduleImagesForLoading(const QList<QUrl> imagesToSchedule)
{
    if (imagesToSchedule.isEmpty())
        return;

    if (d->imageLoadingTotalCount == 0)
    {
        statusBar()->addWidget(d->progressBar);
        d->imageLoadingBunchTimer->start(100);
    }

    d->imageLoadingTotalCount+=imagesToSchedule.count();
    d->progressBar->setRange(0, d->imageLoadingTotalCount);
    d->progressBar->setValue(d->imageLoadingCurrentCount);
    QFutureWatcher<MyImageData>* const watcher = new QFutureWatcher<MyImageData>(this);

    connect(watcher, SIGNAL(resultsReadyAt(int,int)),
            this, SLOT(slotFutureResultsReadyAt(int,int)));

    QFuture<MyImageData> future = QtConcurrent::mapped(imagesToSchedule, LoadImageData);
    watcher->setFuture(future);

    d->imageLoadingRunningFutures << future;
    d->imageLoadingFutureWatchers << watcher;
}

void MainWindow::slotImageLoadingBunchReady()
{
    kDebug()<<"slotImageLoadingBunchReady";

    for (int i = 0; i < d->imageLoadingBuncher.count(); ++i)
    {
        const MyImageData& currentInfo = d->imageLoadingBuncher.at(i);

        // add the item to the tree widget:
        QTreeWidgetItem* const treeItem = new MyImageItem(currentInfo.url, currentInfo.coordinates);
        d->treeWidget->addTopLevelItem(treeItem);
    }

    d->imageLoadingBuncher.clear();

    if (d->imageLoadingTotalCount == 0)
    {
        // remove the QFutures:
        qDeleteAll(d->imageLoadingFutureWatchers);
        d->imageLoadingFutureWatchers.clear();
        d->imageLoadingRunningFutures.clear();
    }
}

void MainWindow::slotMarkersMoved(const QList<QPersistentModelIndex>& markerIndices)
{
    // prepare altitude lookups
    LookupAltitude::Request::List altitudeQueries;

    for (int i = 0; i < markerIndices.count(); ++i)
    {
        const QPersistentModelIndex currentIndex = markerIndices.at(i);
        const GeoCoordinates newCoordinates      = currentIndex.data(RoleCoordinates).value<GeoCoordinates>();

        LookupAltitude::Request myLookup;
        myLookup.coordinates = newCoordinates;
        myLookup.data        = QVariant::fromValue(QPersistentModelIndex(currentIndex));
        altitudeQueries << myLookup;
    }

    if (!altitudeQueries.isEmpty())
    {
        LookupAltitude* const myAltitudeLookup = LookupFactory::getAltitudeLookup("geonames", this);

        connect(myAltitudeLookup, SIGNAL(signalRequestsReady(QList<int>)),
                this, SLOT(slotAltitudeRequestsReady(QList<int>)));

        connect(myAltitudeLookup, SIGNAL(signalDone()),
                this, SLOT(slotAltitudeLookupDone()));

        myAltitudeLookup->addRequests(altitudeQueries);

        d->lookupAltitudeList << myAltitudeLookup;

        /// @todo Check the return value?
        myAltitudeLookup->startLookup();
        kDebug()<<"Starting lookup for "<<altitudeQueries.count()<<" items!";
    }
}

void MainWindow::slotAltitudeRequestsReady(const QList<int>& readyRequests)
{
    kDebug()<<readyRequests.count() << " items ready!";
    LookupAltitude* const myAltitudeLookup = qobject_cast<LookupAltitude*>(sender());

    if (!myAltitudeLookup)
    {
        return;
    }

    for (int i = 0; i < readyRequests.count(); ++i)
    {
        const KGeoMap::LookupAltitude::Request& myLookup = myAltitudeLookup->getRequest(readyRequests.at(i));
        const QPersistentModelIndex markerIndex          = myLookup.data.value<QPersistentModelIndex>();

        if (!markerIndex.isValid())
        {
            continue;
        }

        /// @todo Why does the index return a const model???
        const QAbstractItemModel* const itemModel = markerIndex.model();
        const_cast<QAbstractItemModel*>(itemModel)->setData(markerIndex, QVariant::fromValue(myLookup.coordinates), RoleCoordinates);
    }
}

void MainWindow::slotAltitudeLookupDone()
{
    LookupAltitude* const myAltitudeLookup = qobject_cast<LookupAltitude*>(sender());

    if (!myAltitudeLookup)
    {
        return;
    }

    d->lookupAltitudeList.removeOne(myAltitudeLookup);
    myAltitudeLookup->deleteLater();
}

void MainWindow::slotAddImages()
{
    const QList<QUrl> fileNames = KFileDialog::getOpenUrls(d->lastImageOpenDir, QLatin1String("*.jpg|*.jpeg|*.png"), this, i18n("Add image files"));

    if (fileNames.isEmpty())
        return;

    d->lastImageOpenDir        = fileNames.first().resolved(QUrl("../"));

    slotScheduleImagesForLoading(fileNames);
}

void MainWindow::createMenus()
{
    QMenu* const fileMenu         = menuBar()->addMenu(i18n("File"));
    QAction* const addFilesAction = new QAction(i18n("Add images..."), fileMenu);
    fileMenu->addAction(addFilesAction);

    connect(addFilesAction, SIGNAL(triggered()),
            this, SLOT(slotAddImages()));

    menuBar()->addMenu(helpMenu());
}

KGeoMap::ModelHelper::Flags MarkerModelHelper::modelFlags() const
{
    return FlagMovable;
}
