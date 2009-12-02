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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

// KDE includes

#include <klocale.h>
#include <klineedit.h>
#include <kiconloader.h>

// local includes

#include "mainwindow.h"
#include "worldmapwidget2.h"

using namespace WMW2;

class MainWindowPrivate
{
public:
    MainWindowPrivate()
    : mapWidget(0),
      backendSelector(0),
      inputLatitude(0),
      inputLongitude(0),
      buttonGo(0)
    {

    }
    
    WorldMapWidget2* mapWidget;
    QComboBox* backendSelector;
    KLineEdit* inputLatitude;
    KLineEdit* inputLongitude;
    QPushButton* buttonGo;
    
};

MainWindow::MainWindow(QWidget* const parent)
: QWidget(parent), d(new MainWindowPrivate())
{
    resize(512, 512);
    setWindowTitle(i18n("WorldMapWidget2 demo"));
    setWindowIcon(SmallIcon("applications-internet"));

    QVBoxLayout* const vbox = new QVBoxLayout(this);

    d->mapWidget = new WorldMapWidget2(this);
    vbox->addWidget(d->mapWidget);
    d->mapWidget->setBackend("marble");

    // backend selector
    QWidget* dummyWidget = new QWidget(this);
    QHBoxLayout* hbox = new QHBoxLayout(dummyWidget);
    vbox->addWidget(dummyWidget);
    
    hbox->addWidget(new QLabel(i18n("Backend:")));
    d->backendSelector = new QComboBox(this);
    hbox->addWidget(d->backendSelector);
    const QStringList backends = d->mapWidget->availableBackends();
    QString backendName;
    foreach(backendName, backends)
    {
        d->backendSelector->addItem(backendName);
    }

    hbox->addStretch();

    connect(d->backendSelector, SIGNAL(currentIndexChanged(const QString&)),
            this, SLOT(slotBackendSelectionChanged(const QString&)));

    slotBackendSelectionChanged(d->backendSelector->currentText());

    d->mapWidget->setCenter(WMWGeoCoordinate(51.1, 6.9));
}

MainWindow::~MainWindow()
{
    delete d;
}

void MainWindow::slotBackendSelectionChanged(const QString& newBackendName)
{
    d->mapWidget->setBackend(newBackendName);
}



