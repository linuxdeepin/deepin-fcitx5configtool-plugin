/*
* Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
*
* Author:     liuwenhao <liuwenhao@uniontech.com>
*
* Maintainer: liuwenhao <liuwenhao@uniontech.com>
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

#ifndef KEYLABEL_H
#define KEYLABEL_H

#include <QLabel>
#include <DLabel>
#include <DFrame>
#include <QVBoxLayout>
#include <QPaintEvent>
#include <DApplicationHelper>
#include "publisher/publisherfunc.h"

using namespace Dtk::Widget;
namespace dcc_fcitx_configtool {
namespace widgets {
class FcitxKeyLabel : public DFrame
{
public:
    FcitxKeyLabel(QString text, QWidget *parent = nullptr)
        : DFrame(parent)
    {
        label = new DLabel(text);
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(layout->contentsMargins().left(), 0, layout->contentsMargins().right(), 0);
        layout->setSpacing(0);
        label->setForegroundRole(DPalette::LightLively);
        layout->addWidget(label);
    }

    QString text() { return label->text(); }

protected:
    void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event)
        QStyleOptionFrame opt;
        initStyleOption(&opt);
        QPainter p(this);
        drawShadow(&p, event->rect() - contentsMargins(), QColor(0, 0, 0, 20));

        opt.features |= QStyleOptionFrame::Rounded;

        const DPalette &dp = DApplicationHelper::instance()->palette(this);

        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            p.setBackground(QColor(255, 255, 255));
        } else {
            p.setBackground(QColor(40, 40, 40));
        }

        p.setPen(QPen(dp.frameBorder(), opt.lineWidth));
        style()->drawControl(QStyle::CE_ShapedFrame, &opt, &p, this);
    }

    void drawShadow(QPainter *p, const QRect &rect, const QColor &color) const
    {
        DStyle dstyle;
        int frame_radius = dstyle.pixelMetric(DStyle::PM_FrameRadius);
        int shadow_xoffset = dstyle.pixelMetric(DStyle::PM_ShadowHOffset);
        int shadow_yoffset = dstyle.pixelMetric(DStyle::PM_ShadowVOffset);

        QRect shadow = rect;
        QPoint pointOffset(rect.center().x() + shadow_xoffset, rect.center().y() + shadow_yoffset);
        shadow.moveCenter(pointOffset);

        p->setBrush(color);
        p->setPen(Qt::NoPen);
        p->setRenderHint(QPainter::Antialiasing);
        p->drawRoundedRect(shadow, frame_radius, frame_radius);
    }

private:
    DLabel *label;
};
} // namespace widgets
} // namespace dcc_fcitx_configtool
#endif // KEYLABEL_H
