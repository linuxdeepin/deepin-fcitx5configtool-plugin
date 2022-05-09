/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef CATEGORYHELPER_H
#define CATEGORYHELPER_H

#include <QPainter>
#include <QStyleOptionViewItem>
#include "fcitxqtdbustypes.h"

namespace fcitx {
namespace addim {

void paintCategoryHeader(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index);

QSize categoryHeaderSizeHint();

}
}

#endif
