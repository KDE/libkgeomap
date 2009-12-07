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
#include <QCloseEvent>

// KDE includes

#include <klocale.h>
#include <klineedit.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kconfiggroup.h>

// local includes

#include "mainwindow.h"
#include "worldmapwidget2.h"

using namespace WMW2;

class MainWindowPrivate
{
public:
    MainWindowPrivate()
    : mapWidget(0)
    {

    }

    WorldMapWidget2* mapWidget;
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
    vbox->addWidget(d->mapWidget->getControlWidget());

    readSettings();
}

MainWindow::~MainWindow()
{
    delete d;
}

void MainWindow::readSettings()
{
    KConfig config("worldmapwidget2-demo-1");
    const KConfigGroup groupWidgetConfig = config.group(QString("WidgetConfig"));
    d->mapWidget->readSettingsFromGroup(&groupWidgetConfig);

}

void MainWindow::saveSettings()
{
    KConfig config("worldmapwidget2-demo-1");
    KConfigGroup groupWidgetConfig = config.group(QString("WidgetConfig"));
    d->mapWidget->saveSettingsToGroup(&groupWidgetConfig);
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
        return;

    saveSettings();
    e->accept();
}

