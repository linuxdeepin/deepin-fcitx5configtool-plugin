
/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
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

#include "settingsitem.h"

#include <DPalette>
#include <DStyle>

#include <QStyle>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QLabel>
#include <QApplication>
#include <DStyle>
#include <DApplication>
#include <DGuiApplicationHelper>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

namespace dcc_fcitx_configtool {
namespace widgets {

FcitxSettingsItem::FcitxSettingsItem(QWidget *parent)
    : QFrame(parent)
{
}

bool FcitxSettingsItem::isErr() const
{
    return m_isErr;
}

void FcitxSettingsItem::setIsErr(const bool err)
{
    if (m_isErr == err)
        return;
    m_isErr = err;

    style()->unpolish(this);
    style()->polish(this);
}

void FcitxSettingsItem::addBackground()
{
    //加入一个 DFrame 作为圆角背景
    if (m_bgGroup)
        m_bgGroup->deleteLater();
    m_bgGroup = new DFrame(this);
    m_bgGroup->setBackgroundRole(DPalette::ItemBackground);
    m_bgGroup->setLineWidth(0);
    DStyle::setFrameRadius(m_bgGroup, 8);

    //将 m_bgGroup 沉底
    m_bgGroup->lower();
    //设置m_bgGroup 的大小
    m_bgGroup->setFixedSize(size());
}

void FcitxSettingsItem::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);

    //设置m_bgGroup 的大小
    if (m_bgGroup)
        m_bgGroup->setFixedSize(size());
}

void FcitxSettingsItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if(!m_neetPaint) {
        return;
    }

    QPainter painter( this);
    painter.fillRect(this->rect(), DGuiApplicationHelper::instance()->applicationPalette().window());
    const int radius = 8;
    QRect paintRect = this->rect();
    QPainterPath path;
    if(m_index == firstItem) {
        path.moveTo(paintRect.bottomRight());
        path.lineTo(paintRect.topRight() + QPoint(0, radius));
        path.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)),
                         QSize(radius * 2, radius * 2)), 0, 90);
        path.lineTo(paintRect.topLeft() + QPoint(radius, 0));
        path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
        path.lineTo(paintRect.bottomLeft());
        path.lineTo(paintRect.bottomRight());
    } if(m_index == lastItem) {
        path.moveTo(paintRect.bottomRight() - QPoint(0, radius));
        path.lineTo(paintRect.topRight());
        path.lineTo(paintRect.topLeft());
        path.lineTo(paintRect.bottomLeft() - QPoint(0, radius));
        path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)),
                         QSize(radius * 2, radius * 2)), 180, 90);
        path.lineTo(paintRect.bottomLeft() + QPoint(radius, 0));
        path.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)),
                         QSize(radius * 2, radius * 2)), 270, 90);
    } if(m_index == otherItem) {
        path.moveTo(paintRect.bottomRight());
        path.lineTo(paintRect.topRight());
        path.lineTo(paintRect.topLeft());
        path.lineTo(paintRect.bottomLeft());
        path.lineTo(paintRect.bottomRight());
    } if(m_index == onlyoneItem) {
        path.moveTo(paintRect.bottomRight() - QPoint(0, radius));
        path.lineTo(paintRect.topRight() + QPoint(0, radius));
        path.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)),
                         QSize(radius * 2, radius * 2)), 0, 90);
        path.lineTo(paintRect.topLeft() + QPoint(radius, 0));
        path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
        path.lineTo(paintRect.bottomLeft() - QPoint(0, radius));
        path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)),
                         QSize(radius * 2, radius * 2)), 180, 90);
        path.lineTo(paintRect.bottomLeft() + QPoint(radius, 0));
        path.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)),
                         QSize(radius * 2, radius * 2)), 270, 90);
    }
    if(m_isSelected) {
        QColor color = DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
        if(isDraged()) {
            color.setAlpha(160);
        }
        painter.fillPath(path, color);
    } else if(m_isEntered && (!m_isSelected)) {
        QColor color = DGuiApplicationHelper::instance()->applicationPalette().light().color();
        if(isDraged()) {
            color.setAlpha(160);
        }
        painter.fillPath(path, color);
    } else {
        DPalette p;
        QColor color = Qt::red;
        if(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
            color = QColor("#323232");
            color.setAlpha(230);
        } else {
            color = DGuiApplicationHelper::instance()->applicationPalette().frameBorder().color();
        }
        if(isDraged()) {
            color = DGuiApplicationHelper::instance()->applicationPalette().light().color();
            color.setAlpha(160);
        }
        painter.fillPath(path, color);
    }
   // return FcitxSettingsItem::paintEvent(event);
}

bool FcitxSettingsItem::isEntered() const
{
    return m_isEntered;
}

void FcitxSettingsItem::setIsEntered(bool isEntered)
{
    m_isEntered = isEntered;
}

bool FcitxSettingsItem::isSelected() const
{
    return m_isSelected;
}

void FcitxSettingsItem::setIsSelected(bool isSelected)
{
    m_isSelected = isSelected;
}

} // namespace widgets
} // namespace dcc
