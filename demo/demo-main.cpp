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
#include <kicon.h>

// local includes

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    KAboutData aboutData(
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

    aboutData.addAuthor(ki18n("Michael G. Hansen"),
                         ki18n("WorldMapWidget2 library"),
                         "mike@mghansen.de",
                         "http://www.mghansen.de/");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options;
    options.add( "demopoints_single", ki18n("Add built-in demo points as single markers"));
    options.add( "demopoints_group", ki18n("Add built-in demo points as groupable markers"));
    options.add( "single", ki18n("Do not group the displayed images"));
    options.add( "+[images]", ki18n("List of images") );
    KCmdLineArgs::addCmdLineOptions( options );

    KCmdLineArgs* const args = KCmdLineArgs::parsedArgs();
    
    // get the list of images to load on startup:
    KUrl::List imagesList;
    for (int i=0; i<args->count(); ++i)
    {
        const KUrl argUrl = args->url(i);
//         kDebug()<<argUrl;
        imagesList << argUrl;
    }

    KApplication app;

    MainWindow* myMainWindow = new MainWindow(args);
    myMainWindow->show();
    myMainWindow->slotScheduleImagesForLoading(imagesList);

    return app.exec();
}


