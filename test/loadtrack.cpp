/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2014-07-02
 * @brief  Simple program to load a track for timing tests.
 *
 * @author Copyright (C) 2014 by Michael G. Hansen
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

// Qt includes

#include <QDateTime>
#include <QTextStream>

// KDE includes

#include <qtest_kde.h>
#include <kdebug.h>
#include <kurl.h>

// local includes

#include "../libkgeomap/tracks.h"
#include "../libkgeomap/track_reader.h"

using namespace KGeoMap;

namespace
{
    QTextStream qout(stdout);
    QTextStream qerr(stderr);
}

/**
 * @brief Return the path of the directory containing the test data
 */
KUrl GetTestDataDirectory()
{
    // any better ideas on how to get the path?
    const KUrl thisCPPFile(__FILE__);
    KUrl testDataDir = thisCPPFile.upUrl();
    testDataDir.addPath("data/");
    return testDataDir;
}

/**
 * @brief Test loading of a GPX file directly
 */
bool testSaxLoader(const QString& filename)
{
    const KUrl testDataDir = GetTestDataDirectory();

    TrackReader::TrackReadResult fileData = TrackReader::loadTrackFile(KUrl(filename));

    return fileData.isValid;
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    if (argc<2)
    {
        qerr << QLatin1String("Need a filename as argument to load") << endl;
        return 1;
    }

    const QString filename = argv[1];
    qerr << QString("Loading file: %1").arg(filename) << endl;
    const bool success = testSaxLoader(filename);
    if (!success)
    {
        qerr << "Loading failed" << endl;
        return 1;
    }

    qerr << "Loaded successfully." << endl;
    return 0;
}
