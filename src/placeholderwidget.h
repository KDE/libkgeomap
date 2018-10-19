/** ===========================================================
 * @file
 *
 * This file is a part of KDE project
 *
 *
 * @date   2010-12-05
 * @brief  Placeholder widget for when backends are activated
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
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

#ifndef KGEOMAP_PLACEHOLDERWIDGET_H
#define KGEOMAP_PLACEHOLDERWIDGET_H

// Qt includes

#include <QtWidgets/QFrame>

namespace KGeoMap
{

class PlaceholderWidget : public QFrame
{
    Q_OBJECT

public:

    explicit PlaceholderWidget(QWidget* const parent = nullptr);
    ~PlaceholderWidget() override;

    void setMessage(const QString& message);

private:

    Q_DISABLE_COPY(PlaceholderWidget)

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace KGeoMap

#endif // KGEOMAP_PLACEHOLDERWIDGET_H
