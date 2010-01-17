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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// KDE includes

#include <kmainwindow.h>
#include <kurl.h>

class MainWindowPrivate;

class MainWindow : public KMainWindow
{
Q_OBJECT

public:
    MainWindow(QWidget* const parent = 0);
    ~MainWindow();

public Q_SLOTS:
    void slotScheduleImagesForLoading(const KUrl::List imagesToSchedule);

protected:
    void readSettings();
    void saveSettings();
    void closeEvent(QCloseEvent* e);

private Q_SLOTS:
    void slotFutureResultsReadyAt(int startIndex, int endIndex);
    void slotImageLoadingBunchReady();

private:
    MainWindowPrivate* const d;
};

#endif /* MAINWINDOW_H */
