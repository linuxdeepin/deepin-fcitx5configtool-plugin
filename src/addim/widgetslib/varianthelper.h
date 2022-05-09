/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef VARIANTHELPER_H
#define VARIANTHELPER_H

#include <QString>
#include <QVariantMap>

namespace fcitx {
namespace addim {

QVariant readVariant(const QVariant &value, const QString &path);

bool readBool(const QVariantMap &map, const QString &path);

QString readString(const QVariantMap &map, const QString &path);

void writeVariant(QVariantMap &map, const QString &path, const QVariant &value);

}
}

#endif
