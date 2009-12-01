/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : demo-program for WorldMapWidget2
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


// KDE includes

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KDebug>

// local includes

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    const KAboutData aboutData(
    "demo-worldmapwidget2",
    0,
    ki18n("WorldMapWidget2 demo application"),
    "0.1", // version
    ki18n("Presents the WorldMapWidget2"),
    KAboutData::License_GPL,
    ki18n("(c) 2009 Michael G. Hansen"),
    ki18n(""), // optional text
    "", // URI of homepage
    "" // bugs e-mail address
  );

  KCmdLineArgs::init(argc, argv, &aboutData);

  KApplication app;

  MainWindow myMainWindow;
  myMainWindow.show();

  return app.exec();
}


