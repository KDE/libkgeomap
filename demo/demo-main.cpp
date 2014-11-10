/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  demo-program for KGeoMap
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

// KDE includes

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kicon.h>

// libkgeomap includes

#include "libkgeomap/version.h"

// local includes

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    KAboutData aboutData("demo-kgeomap",
                         0,
                         ki18n("KGeoMap demo application"),
                         kgeomap_version,                                   // version
                         ki18n("Presents the World Map Widget Interface"),
                         KAboutData::License_GPL,
                         ki18n("(c) 2009-2010 Michael G. Hansen"),
                         ki18n(""),                                         // optional text
                         "http://www.digikam.org/sharedlibs",               // URI of homepage
                         ""                                                 // bugs e-mail address
                        );

    aboutData.addAuthor(ki18n("Michael G. Hansen"),
                        ki18n("KGeoMap library"),
                        "mike@mghansen.de",
                        "http://www.mghansen.de");

    aboutData.addCredit(ki18n("Justus Schwartz"),
                        ki18n("Patch for displaying tracks on the map."),
                              "justus at gmx dot li");

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

    for (int i=0; i < args->count(); ++i)
    {
        const KUrl argUrl = args->url(i);
//         kDebug()<<argUrl;
        imagesList << argUrl;
    }

    KApplication app;

    MainWindow* const myMainWindow = new MainWindow(args);
    myMainWindow->show();
    myMainWindow->slotScheduleImagesForLoading(imagesList);

    return app.exec();
}
