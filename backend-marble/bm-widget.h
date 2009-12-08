/* ============================================================
 *
 * Date        : 2009-12-08
 * Description : Marble-backend for WorldMapWidget2
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

#ifndef BM_MARBLE_H
#define BM_MARBLE_H

// KDE includes

#include <marble/MarbleWidget.h>

namespace WMW2 {

class BackendMarble;

class BMWidget : public Marble::MarbleWidget
{
Q_OBJECT

public:
    BMWidget(BackendMarble* const pMarbleBackend, QWidget* const parent = 0);
    virtual ~BMWidget();

protected:
    virtual void customPaint(Marble::GeoPainter* painter);

private:
    BackendMarble* const marbleBackend;
};

} /* WMW2 */

#endif /* BM_MARBLE_H */

