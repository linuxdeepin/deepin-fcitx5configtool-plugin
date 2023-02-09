// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QEvent>
#include <QScrollBar>
#include <QVBoxLayout>

#include "verticalscrollarea.h"


VerticalScrollArea::VerticalScrollArea(QWidget *parent) : QScrollArea(parent) {
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void VerticalScrollArea::setWidget(QWidget *widget) {
    QScrollArea::setWidget(widget);
    widget->installEventFilter(this);
}

bool VerticalScrollArea::eventFilter(QObject *o, QEvent *e) {
    if (o == widget() && e->type() == QEvent::Resize)
        setMinimumWidth(widget()->minimumSizeHint().width() +
                        verticalScrollBar()->width());

    return false;
}
