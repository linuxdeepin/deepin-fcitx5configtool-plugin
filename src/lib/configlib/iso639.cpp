/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "iso639.h"
#include "config.h"
#include "logging.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace fcitx {
namespace kcm {
namespace {

QMap<QString, QString> readAlpha3ToNameMap(const char *name, const char *base) {
    QMap<QString, QString> map;
    QFile file(name);
    qCDebug(KCM_FCITX5) << "Opening ISO code file:" << name;
    file.open(QIODevice::ReadOnly);
    auto data = file.readAll();
    QJsonParseError error;
    auto document = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        qCWarning(KCM_FCITX5) << "Failed to parse JSON from file:" << name
                  << "error:" << error.errorString();
        return {};
    }

    auto object = document.object();
    auto iso = object.value(base);
    if (!iso.isArray()) {
        return {};
    }
    const auto array = iso.toArray();
    qCDebug(KCM_FCITX5) << "Processing" << base << "array with" << array.size() << "items";
    int validCount = 0;
    for (const auto &item : array) {
        if (!item.isObject()) {
            qCDebug(KCM_FCITX5) << "Skipping non-object item in array";
            continue;
        }
        auto alpha3 = item.toObject().value("alpha_3").toString();
        auto bibliographic = item.toObject().value("bibliographic").toString();
        auto name = item.toObject().value("name").toString();
        if (alpha3.isEmpty() || name.isEmpty()) {
            qCDebug(KCM_FCITX5) << "Skipping item with empty alpha3 or name";
            continue;
        }
        map.insert(alpha3, name);
        validCount++;
        if (!bibliographic.isEmpty()) {
            map.insert(bibliographic, name);
            validCount++;
        }
    }
    qCDebug(KCM_FCITX5) << "Loaded" << validCount << "valid entries from" << base;
    return map;
}
} // namespace

Iso639::Iso639() {
    qCDebug(KCM_FCITX5) << "Initializing ISO 639 data";
    iso639_2data_ = readAlpha3ToNameMap(ISOCODES_ISO639_2_JSON, "639-2");
    iso639_3data_ = readAlpha3ToNameMap(ISOCODES_ISO639_3_JSON, "639-3");
    iso639_5data_ = readAlpha3ToNameMap(ISOCODES_ISO639_5_JSON, "639-5");
    qCDebug(KCM_FCITX5) << "ISO 639 data initialized - 639-2:" << iso639_2data_.size()
           << "entries, 639-3:" << iso639_3data_.size()
           << "entries, 639-5:" << iso639_5data_.size() << "entries";
}

} // namespace kcm
} // namespace fcitx
