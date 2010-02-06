/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : Widget for displaying HTML in the backends
 *
 * Copyright (C) 2009,2010 by Michael G. Hansen <mike at mghansen dot de>
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

// Qt includes

#include <QTimer>

// KDE includes

#include <khtmlview.h>

// local includes

#include "html_widget.h"


namespace WMW2 {

class HTMLWidgetPrivate
{
public:
    HTMLWidgetPrivate()
    : parent(0),
      isReady(false),
      javascriptScanTimer(0)
    {
    }

    QWidget* parent;
    bool isReady;
    QTimer* javascriptScanTimer;
};

HTMLWidget::HTMLWidget(QWidget* const parent)
: KHTMLPart(parent), d(new HTMLWidgetPrivate())
{
    d->parent = parent;
    
    widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // TODO: the khtmlpart-widget does not resize automatically, we have to do it manually???
    parent->installEventFilter(this);

    // create a timer for monitoring for javascript events, but do not start it yet:
    d->javascriptScanTimer = new QTimer(this);
    d->javascriptScanTimer->setSingleShot(false);
    d->javascriptScanTimer->setInterval(300);
    connect(d->javascriptScanTimer, SIGNAL(timeout()),
            this, SLOT(slotScanForJSMessages()));

    connect(this, SIGNAL(completed()),
            this, SLOT(slotHTMLCompleted()));
}

HTMLWidget::~HTMLWidget()
{
    delete d;
}

void HTMLWidget::loadInitialHTML(const QString& initialHTML)
{
//     kDebug()<<initialHTML;
    begin();
    write(initialHTML);
    end();
}

bool HTMLWidget::eventFilter(QObject* object, QEvent* event)
{
    if (object==d->parent)
    {
        if (event->type()==QEvent::Resize)
        {
            QResizeEvent* const resizeEvent = dynamic_cast<QResizeEvent*>(event);
            if (resizeEvent)
            {
                widget()->resize(resizeEvent->size());
                view()->resize(resizeEvent->size());

                // TODO: the map div does not adjust its height properly if height=100%,
                //       therefore we adjust it manually here
                if (d->isReady)
                {
                    runScript(QString("wmwWidgetResized(%1, %2)").arg(d->parent->width()).arg(d->parent->height()));
                }
            }
        }
    }
    return false;
}

void HTMLWidget::slotHTMLCompleted()
{
    d->isReady = true;
    runScript(QString("wmwWidgetResized(%1, %2)").arg(d->parent->width()).arg(d->parent->height()));

    // start monitoring for javascript events using a timer:
    d->javascriptScanTimer->start();

    emit(signalJavaScriptReady());
}

void HTMLWidget::khtmlMousePressEvent(khtml::MousePressEvent* e)
{
    slotScanForJSMessages();
    KHTMLPart::khtmlMousePressEvent(e);
}

void HTMLWidget::khtmlMouseReleaseEvent(khtml::MouseReleaseEvent* e)
{
    slotScanForJSMessages();
    KHTMLPart::khtmlMouseReleaseEvent(e);
}

void HTMLWidget::khtmlMouseMoveEvent(khtml::MouseMoveEvent *e)
{
    slotScanForJSMessages();
    KHTMLPart::khtmlMouseMoveEvent(e);
}

void HTMLWidget::slotScanForJSMessages()
{
    const QString status = jsStatusBarText();

    if (status!="(event)")
        return;

    kDebug()<<status;
    const QString eventBufferString = runScript(QString("wmwReadEventStrings();")).toString();
    if (eventBufferString.isEmpty())
        return;

    const QStringList events = eventBufferString.split('|');

    emit(signalHTMLEvents(events));
}

/**
 * @brief Wrapper around executeScript to catch more errors
 */
QVariant HTMLWidget::runScript(const QString& scriptCode)
{
    WMW2_ASSERT(d->isReady);

    if (!d->isReady)
        return QVariant();

    kDebug()<<scriptCode;
    return executeScript(scriptCode);
}

} /* WMW2 */

