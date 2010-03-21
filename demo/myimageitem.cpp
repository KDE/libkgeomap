/* ============================================================
 *
 * Date        : 2010-03-20
 * Description : sub class of QStandardItem to represent the images
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
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

#include "myimageitem.h"

MyImageItem::MyImageItem(const KUrl& url, const WMW2::WMWGeoCoordinate& itemCoordinates)
: QTreeWidgetItem(), coordinates(itemCoordinates), imageUrl(url)
{
}

MyImageItem::~MyImageItem()
{
}

QVariant MyImageItem::data(int column, int role) const
{
    if (role==RoleCoordinates)
    {
        return QVariant::fromValue(coordinates);
    }
    else if (role==Qt::DisplayRole)
    {
        switch (column)
        {
            case 0:
                return imageUrl.fileName();

            case 1:
                return coordinates.geoUrl();

            default:
                return QVariant();
        }
    }

    return QTreeWidgetItem::data(column, role);
}

void MyImageItem::setData(int column, int role, const QVariant& value)
{
    if (role==RoleCoordinates)
    {
        if (value.canConvert<WMW2::WMWGeoCoordinate>())
        {
            coordinates = value.value<WMW2::WMWGeoCoordinate>();
            emitDataChanged();
        }
        return;
    }

    QTreeWidgetItem::setData(column, role, value);
}

