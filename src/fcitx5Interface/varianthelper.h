// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VARIANTHELPER_H_
#define VARIANTHELPER_H_

#include <QString>
#include <QVariantMap>


QVariant readVariant(const QVariant &value, const QString &path);

bool readBool(const QVariantMap &map, const QString &path);

QString readString(const QVariantMap &map, const QString &path);

void writeVariant(QVariantMap &map, const QString &path, const QVariant &value);


#endif // VARIANTHELPER_H_
