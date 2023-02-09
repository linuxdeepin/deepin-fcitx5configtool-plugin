// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CATEGORYHELPER_H
#define CATEGORYHELPER_H

#include <QPainter>
#include <QStyleOptionViewItem>
#include "fcitxqtdbustypes.h"

namespace fcitx {
namespace addim {

void setSelectCategoryRow(int selectRow);

void paintCategoryHeader(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index);

QSize categoryHeaderSizeHint();

}
}

#endif
