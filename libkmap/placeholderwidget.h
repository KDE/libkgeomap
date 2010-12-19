/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
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

#ifndef PLACEHOLDERWIDGET_H
#define PLACEHOLDERWIDGET_H

// Qt includes

#include <QFrame>

namespace KMap
{

class PlaceholderWidget : public QFrame
{
    Q_OBJECT

public:
    PlaceholderWidget(QWidget* parent = 0);
    ~PlaceholderWidget();

    void setMessage(const QString& message);

private:

    Q_DISABLE_COPY(PlaceholderWidget)

    class PlaceholderWidgetPrivate;
    PlaceholderWidgetPrivate* const d;

};

} /* KMap */

#endif /* PLACEHOLDERWIDGET_H */
