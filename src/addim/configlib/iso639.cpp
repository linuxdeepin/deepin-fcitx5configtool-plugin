// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iso639.h"
#include "config.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace fcitx {
namespace addim {
namespace {

QMap<QString, QString> readAlpha3ToNameMap(const char *name, const char *base) {
    QMap<QString, QString> map;
    QFile file(name);
    file.open(QIODevice::ReadOnly);
    auto data = file.readAll();
    QJsonParseError error;
    auto document = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        return {};
    }

    auto object = document.object();
    auto iso = object.value(base);
    if (!iso.isArray()) {
        return {};
    }
    const auto array = iso.toArray();
    for (const auto &item : array) {
        if (!item.isObject()) {
            continue;
        }
        auto alpha3 = item.toObject().value("alpha_3").toString();
        auto bibliographic = item.toObject().value("bibliographic").toString();
        auto name = item.toObject().value("name").toString();
        if (alpha3.isEmpty() || name.isEmpty()) {
            continue;
        }
        map.insert(alpha3, name);
        if (!bibliographic.isEmpty()) {
            map.insert(bibliographic, name);
        }
    }
    return map;
}
} // namespace

Iso639::Iso639() {
    m_iso639_2data = readAlpha3ToNameMap(ISOCODES_ISO639_2_JSON, "639-2");
    m_iso639_3data = readAlpha3ToNameMap(ISOCODES_ISO639_3_JSON, "639-3");
    m_iso639_5data = readAlpha3ToNameMap(ISOCODES_ISO639_5_JSON, "639-5");
}

} // namespace addim
} // namespace fcitx
