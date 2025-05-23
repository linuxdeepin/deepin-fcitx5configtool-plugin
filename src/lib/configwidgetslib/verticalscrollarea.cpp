/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <QEvent>
#include <QScrollBar>
#include <QVBoxLayout>

#include "verticalscrollarea.h"
#include "logging.h"

namespace fcitx {
namespace kcm {

VerticalScrollArea::VerticalScrollArea(QWidget *parent) : QScrollArea(parent) {
    qCDebug(KCM_FCITX5) << "Creating VerticalScrollArea";
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    qCDebug(KCM_FCITX5) << "VerticalScrollArea initialized with no frame and resizable widget";
}

void VerticalScrollArea::setWidget(QWidget *widget) {
    qCDebug(KCM_FCITX5) << "Setting widget for VerticalScrollArea";
    QScrollArea::setWidget(widget);
    widget->installEventFilter(this);
    qCDebug(KCM_FCITX5) << "Widget set and event filter installed";
}

bool VerticalScrollArea::eventFilter(QObject *o, QEvent *e) {
    if (o == widget() && e->type() == QEvent::Resize) {
        qCDebug(KCM_FCITX5) << "Handling widget resize event";
        int newWidth = widget()->minimumSizeHint().width() + verticalScrollBar()->width();
        setMinimumWidth(newWidth);
        qCDebug(KCM_FCITX5) << "Adjusted minimum width to:" << newWidth;
    }

    return false;
}

} // namespace kcm
} // namespace fcitx
