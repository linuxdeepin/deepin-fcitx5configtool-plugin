/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "varianthelper.h"
#include <QDBusArgument>
#include "logging.h"

namespace fcitx {
namespace kcm {

QVariantMap toMap(const QVariant &variant) {
    QVariantMap map;
    if (variant.canConvert<QDBusArgument>()) {
        qCDebug(KCM_FCITX5) << "Converting QDBusArgument to QVariantMap";
        auto argument = qvariant_cast<QDBusArgument>(variant);
        argument >> map;
    }
    if (variant.canConvert<QVariantMap>()) {
        qCDebug(KCM_FCITX5) << "Converting QVariant to QVariantMap";
        map = variant.toMap();
    }
    qCDebug(KCM_FCITX5) << "Converted map size:" << map.size();
    return map;
}

QString valueFromVariantMapByPath(const QVariantMap &map,
                                  const QStringList &path, int depth) {
    qCDebug(KCM_FCITX5) << "Looking up path:" << path << "at depth:" << depth;
    auto iter = map.find(path[depth]);
    if (iter == map.end()) {
        qCDebug(KCM_FCITX5) << "Path not found:" << path[depth];
        return QString();
    }
    if (depth + 1 == path.size()) {
        if (iter->canConvert<QString>()) {
            QString value = iter->toString();
            qCDebug(KCM_FCITX5) << "Found value:" << value;
            return value;
        }
    } else {
        QVariantMap map = toMap(*iter);

        if (!map.isEmpty()) {
            return valueFromVariantMapByPath(map, path, depth + 1);
        }
    }
    qCDebug(KCM_FCITX5) << "No string value found";
    return QString();
}

QVariant valueFromVariantHelper(const QVariant &value,
                                const QStringList &pathList, int depth) {
    qCDebug(KCM_FCITX5) << "valueFromVariantHelper depth:" << depth;
    if (depth == pathList.size()) {
        qCDebug(KCM_FCITX5) << "Reached target depth, returning value";
        return value;
    }
    auto map = toMap(value);
    if (map.isEmpty() || !map.contains(pathList[depth])) {
        qCDebug(KCM_FCITX5) << "Path not found or empty map:" << pathList[depth];
        return {};
    }
    return valueFromVariantHelper(map[pathList[depth]], pathList, depth + 1);
}

QVariant readVariant(const QVariant &value, const QString &path) {
    qCDebug(KCM_FCITX5) << "readVariant path:" << path;
    auto pathList = path.split("/");
    QVariant result = valueFromVariantHelper(toMap(value), pathList, 0);
    qCDebug(KCM_FCITX5) << "readVariant result:" << result;
    return result;
}

QString readString(const QVariantMap &map, const QString &path) {
    qCDebug(KCM_FCITX5) << "readString path:" << path;
    auto pathList = path.split("/");
    if (pathList.empty()) {
        qCDebug(KCM_FCITX5) << "Empty path";
        return QString();
    }
    QString result = valueFromVariantMapByPath(map, pathList, 0);
    qCDebug(KCM_FCITX5) << "readString result:" << result;
    return result;
}

bool readBool(const QVariantMap &map, const QString &path) {
    qCDebug(KCM_FCITX5) << "readBool path:" << path;
    bool result = readString(map, path) == "True";
    qCDebug(KCM_FCITX5) << "readBool result:" << result;
    return result;
}

void writeVariantHelper(QVariantMap &map, const QStringList &path,
                        const QVariant &value, int depth) {
    qCDebug(KCM_FCITX5) << "writeVariantHelper path:" << path << "depth:" << depth;
    if (depth + 1 == path.size()) {
        qCDebug(KCM_FCITX5) << "Setting value at:" << path[depth];
        map[path[depth]] = value;
    } else {
        auto iter = map.find(path[depth]);
        if (iter == map.end()) {
            qCDebug(KCM_FCITX5) << "Creating new map at:" << path[depth];
            iter = map.insert(path[depth], QVariantMap());
        }

        if (iter->type() != QVariant::Map) {
            qCDebug(KCM_FCITX5) << "Converting to map at:" << path[depth];
            auto oldValue = *iter;
            *iter = QVariantMap({{"", oldValue}});
        }

        auto &nextMap = *static_cast<QVariantMap *>(iter->data());
        writeVariantHelper(nextMap, path, value, depth + 1);
    }
}

void writeVariant(QVariantMap &map, const QString &path,
                  const QVariant &value) {
    qCDebug(KCM_FCITX5) << "writeVariant path:" << path;
    auto pathList = path.split("/");
    if (pathList.empty()) {
        qCDebug(KCM_FCITX5) << "Empty path";
        return;
    }
    writeVariantHelper(map, pathList, value, 0);
    qCDebug(KCM_FCITX5) << "writeVariant completed";
}

} // namespace kcm
} // namespace fcitx
