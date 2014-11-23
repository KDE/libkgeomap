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


#include <KAboutData>

#include <QDebug>
#include <kicon.h>
#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>

// libkgeomap includes

//#include "version.h"

// local includes

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    KAboutData aboutData("demo-kgeomap", i18n("KGeoMap demo application"), "kgeomap_version"); // TODO fix version
    aboutData.setShortDescription(i18n("Presents the World Map Widget Interface"));
    aboutData.setLicense(KAboutLicense::GPL);
    aboutData.setCopyrightStatement(i18n("(c) 2009-2010 Michael G. Hansen"));
    aboutData.setHomepage("http://www.digikam.org/sharedlibs");

    aboutData.addAuthor(i18n("Michael G. Hansen"),
                        i18n("KGeoMap library"),
                        "mike@mghansen.de",
                        "http://www.mghansen.de");

    aboutData.addCredit(i18n("Justus Schwartz"),
                        i18n("Patch for displaying tracks on the map."),
                              "justus at gmx dot li");

    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    //PORTING SCRIPT: adapt aboutdata variable if necessary
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("demopoints_single"), i18n("Add built-in demo points as single markers")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("demopoints_group"), i18n("Add built-in demo points as groupable markers")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("single"), i18n("Do not group the displayed images")));
    parser.addPositionalArgument("images", i18n("List of images"), "[images...]");

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    // get the list of images to load on startup:
    QList<QUrl> imagesList;

    Q_FOREACH(const QString& file, parser.positionalArguments())
    {
        const QUrl argUrl = QUrl::fromLocalFile(file);
        qDebug()<<argUrl;
        imagesList << argUrl;
    }


    MainWindow* const myMainWindow = new MainWindow(&parser);
    myMainWindow->show();
    myMainWindow->slotScheduleImagesForLoading(imagesList);

    return app.exec();
}
