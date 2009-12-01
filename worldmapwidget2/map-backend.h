/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : Base-class for backends for WorldMapWidget2
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

#ifndef MAP_BACKEND_H
#define MAP_BACKEND_H

// Qt includes

#include <QWidget>

namespace WMW2 {

class MapBackendPrivate;

class MapBackend : public QObject
{

Q_OBJECT

public:

    MapBackend(QObject* const parent);
    virtual ~MapBackend();

    virtual QString backendName() const = 0;
    virtual QWidget* mapWidget() const = 0;
    
private:
    MapBackendPrivate* const d;
};

} /* WMW2 */

#endif /* MAP_BACKEND_H */
