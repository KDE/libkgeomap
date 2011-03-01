/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-08-16
 * @brief  Helper class to access models
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
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

#ifndef KMAP_MODELHELPER
#define KMAP_MODELHELPER

// Qt includes

#include <QtCore/QAbstractItemModel>
#include <QtGui/QItemSelectionModel>
#include <QtCore/QPersistentModelIndex>
#include <QtGui/QPixmap>
#include <QtCore/QPoint>
#include <QtCore/QString>

// Kde includes

#include <kdebug.h>
#include <kurl.h>

// local includes

#include "libkmap_export.h"
#include "kmap_primitives.h"

namespace KMap
{


class KMAP_EXPORT ModelHelper : public QObject
{
    Q_OBJECT

public:

    enum Flag
    {
        FlagNull    = 0,
        FlagVisible = 1,
        FlagMovable = 2,
        FlagSnaps   = 4
    };

    Q_DECLARE_FLAGS(Flags, Flag)

    ModelHelper(QObject* const parent = 0);
    virtual ~ModelHelper();

    void snapItemsTo(const QModelIndex& targetIndex, const QList<QPersistentModelIndex>& snappedIndices);

    // these are necessary for grouped and ungrouped models
    virtual QAbstractItemModel* model() const = 0;
    virtual QItemSelectionModel* selectionModel() const = 0;
    virtual bool itemCoordinates(const QModelIndex& index, GeoCoordinates* const coordinates) const = 0;
    virtual Flags modelFlags() const;

    // these are necessary for ungrouped models
    virtual bool itemIcon(const QModelIndex& index, QPoint* const offset, QSize* const size, QPixmap* const pixmap, KUrl* const url) const;
    virtual Flags itemFlags(const QModelIndex& index) const;
    virtual void snapItemsTo(const QModelIndex& targetIndex, const QList<QModelIndex>& snappedIndices);

    // these are used by MarkerModel for grouped models
    virtual QPixmap pixmapFromRepresentativeIndex(const QPersistentModelIndex& index, const QSize& size);
    virtual QPersistentModelIndex bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list, const int sortKey);

    virtual void onIndicesClicked(const QList<QPersistentModelIndex>& clickedIndices);
    virtual void onIndicesMoved(const QList<QPersistentModelIndex>& movedIndices, const GeoCoordinates& targetCoordinates, const QPersistentModelIndex& targetSnapIndex);

Q_SIGNALS:

    void signalVisibilityChanged();
    void signalThumbnailAvailableForIndex(const QPersistentModelIndex& index, const QPixmap& pixmap);
    void signalModelChangedDrastically();
};

} /* namespace KMap */

Q_DECLARE_OPERATORS_FOR_FLAGS(KMap::ModelHelper::Flags)

#endif /* KMAP_MODELHELPER */
