/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "glo.h"
#include "imelog.h"
#include "categoryhelper.h"
#include <QApplication>
#include <QPainterPath>
#include <QColor>
#include <DGuiApplicationHelper>
using namespace Dtk::Gui;

namespace fcitx {
namespace addim {

static constexpr int SPACING = 0;
static constexpr int HEIGHT_EXTEND = 20;

int g_selectRow = -1;

void setSelectCategoryRow(int selectRow)
{
	g_selectRow = selectRow;
}

void paintCategoryHeader(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) {

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    int row = index.row();
    QString category = index.model()->data(index, Qt::DisplayRole).toString();

    QRect optRect = option.rect;
    optRect.translate(SPACING, SPACING);
    optRect.setWidth(optRect.width() - SPACING * 2);
    optRect.setHeight(optRect.height() - SPACING * 2);
#if 1
    QFont font(QApplication::font());
    int point_size = font.pointSize();
    font.setPixelSize(point_size + 2);
    const QFontMetrics fontMetrics = QFontMetrics(font);
#endif

    QColor outlineColor = option.palette.text().color();
    outlineColor.setAlphaF(0.15);

    {
        QRectF rect;
        rect.setX(option.rect.x());
        rect.setY(option.rect.y());
        rect.setWidth(option.rect.width() - 1);
        rect.setHeight(option.rect.height() - 1);

        const qreal radius = 7;
        QPainterPath path;
        path.moveTo(rect.topRight() - QPointF(radius, 0));
        path.lineTo(rect.topLeft() + QPointF(radius, 0));
        path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, radius));
        path.lineTo(rect.bottomLeft() + QPointF(0, -radius));
        path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(radius, 0));
        path.lineTo(rect.bottomRight() - QPointF(radius, 0));
        path.quadTo(rect.bottomRight(), rect.bottomRight() + QPointF(0, -radius));
        path.lineTo(rect.topRight() + QPointF(0, radius));
        path.quadTo(rect.topRight(), rect.topRight() + QPointF(-radius, -0));

        if (option.state.testFlag(QStyle::State_Selected)) {
            painter->fillPath(path, DGuiApplicationHelper::instance()->applicationPalette().light());
        } else if (option.state.testFlag(QStyle::State_MouseOver)) {
            painter->fillPath(path, DGuiApplicationHelper::instance()->applicationPalette().light());
        } else {
            if (g_selectRow != -1 && g_selectRow == row) {
                painter->fillPath(path, DGuiApplicationHelper::instance()->applicationPalette().light());
            } else {
                painter->fillPath(path, DGuiApplicationHelper::instance()->applicationPalette().base());
            }
        }
    }

    {
        QRect textRect(option.rect);
        textRect.setTop(textRect.top() + 8);
        textRect.setLeft(textRect.left() + 8);
        textRect.setHeight(20);
        textRect.setRight(textRect.right() - 8);

        painter->setFont(font);
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
            QColor penColor("#FFFFFF");
            painter->setPen(penColor);
        } else {
            QColor penColor(option.palette.text().color());
            painter->setPen(penColor);
        }
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, category);

    }

    {
        if (option.state & QStyle::State_Selected) {
            QRect radioRec(optRect.right() - 30, optRect.top() + (optRect.height() - 32) / 2, 32, 32);
            QPixmap pix(":/img/check_ok.png");
            painter->drawPixmap(radioRec, pix);
            g_selectRow = row;
        }
        else {
            if (g_selectRow != -1 && g_selectRow == row) {
                QRect radioRec(optRect.right() - 30, optRect.top() + (optRect.height() - 32) / 2, 32, 32);
                QPixmap pix(":/img/check_ok.png");
                painter->drawPixmap(radioRec, pix);
            }
        }
    }

    int useCount = getUseIMLanguageCount();
    if (useCount > 0 && (useCount - 1 == row))
    {
        QPoint start(optRect.bottomLeft());
        painter->fillRect(QRect(start, QSize(optRect.width(), 1)), outlineColor);
    }

    painter->restore();
}

QSize categoryHeaderSizeHint() {
    const int height = 36;
    return QSize(0, height);
}

}
}
