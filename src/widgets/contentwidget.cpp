/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "widgets/contentwidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QResizeEvent>
#include <QTimer>
#include <QDebug>
#include <QScroller>
#include <QScrollBar>
#include <QScrollArea>
#include <QPropertyAnimation>

DWIDGET_USE_NAMESPACE

namespace dcc_fcitx_configtool {
namespace widgets {
FcitxContentWidget::FcitxContentWidget(QWidget *parent)
    : QWidget(parent)
    , m_content(nullptr)
{
    m_contentArea = new QScrollArea;
    m_contentArea->setWidgetResizable(true);
    m_contentArea->setFrameStyle(QFrame::NoFrame);
    m_contentArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_contentArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // m_contentArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    m_contentArea->setContentsMargins(0, 0, 0, 0);

    //QScroller::grabGesture(m_contentArea->viewport(), QScroller::LeftMouseButtonGesture);
    QScroller *scroller = QScroller::scroller(m_contentArea->viewport());
    QScrollerProperties sp;
    sp.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    scroller->setScrollerProperties(sp);

    QVBoxLayout *centralLayout = new QVBoxLayout(this);

    centralLayout->addWidget(m_contentArea);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);

    setLayout(centralLayout);
    setObjectName("FcitxContentWidget");
    setBackgroundRole(QPalette::Base);
}

FcitxContentWidget::~FcitxContentWidget()
{
    QScroller *scroller = QScroller::scroller(m_contentArea->viewport());
    if (scroller) {
        scroller->stop();
    }
}

QWidget *FcitxContentWidget::setContent(QWidget *const w)
{
    QWidget *lastWidget = m_content;

    if (lastWidget)
        lastWidget->removeEventFilter(this);

    m_content = w;
    m_content->installEventFilter(this);
    m_contentArea->setWidget(m_content);

    return lastWidget;
}

void FcitxContentWidget::scrollTo(int dy)
{
    m_contentArea->verticalScrollBar()->setValue(m_contentArea->verticalScrollBar()->value() + dy);
}

void FcitxContentWidget::resizeEvent(QResizeEvent *event)
{
    if (m_content)
        m_content->setFixedWidth(event->size().width());
}

void FcitxContentWidget::mouseMoveEvent(QMouseEvent *event)
{
    static int lastY = event->pos().y();
    int pos = m_contentArea->verticalScrollBar()->sliderPosition();
    if (lastY < event->pos().y()) {
        m_contentArea->verticalScrollBar()->setSliderPosition(pos + 3);
    } else {
        m_contentArea->verticalScrollBar()->setSliderPosition(pos - 3);
    }
    qDebug() << pos;
    lastY = event->pos().y();
}
} // namespace widgets
} // namespace dcc_fcitx_configtool
