// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fcitx5configproxy.h"
#include "private/fcitx5configproxy_p.h"

#include <dbusprovider.h>
#include <varianthelper.h>

#include <QDBusPendingCallWatcher>

using namespace deepin::fcitx5configtool;

Fcitx5ConfigProxyPrivate::Fcitx5ConfigProxyPrivate(Fcitx5ConfigProxy *parent,
                                                 fcitx::kcm::DBusProvider *dbus,
                                                 const QString &path)
    : q(parent), dbusprovider(dbus), path(path)
{
    timer = new QTimer(q);
    timer->setInterval(1000);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, q, &Fcitx5ConfigProxy::save);
}

QStringList Fcitx5ConfigProxyPrivate::formatKey(const QString &shortcut) {
    QStringList list;
    for (const auto &key : shortcut.split("+")) {
        if (key.trimmed().toLower() == "control")
            list << "Ctrl";
        else if (key.trimmed().toLower() == "backspace")
            list << "Backspace";
        else if (key.trimmed().toLower() == "space")
            list << "Space";
        else
            list << key.trimmed();
    }
    return list;
}

QString Fcitx5ConfigProxyPrivate::formatKeys(const QStringList &keys) {
    QStringList list;   
    for (const auto &key : keys) {
        if (key.trimmed().toLower() == "ctrl")
            list << "Control";
        else if (key.trimmed().toLower() == "backspace")
            list << "BackSpace";
        else if (key.trimmed().toLower() == "space")
            list << "space";
        else
            list << key.trimmed();
    }
    return list.join("+");
}

QVariant Fcitx5ConfigProxyPrivate::readDBusValue(const QVariant &value) {
    if (value.canConvert<QDBusArgument>()) {
        auto argument = qvariant_cast<QDBusArgument>(value);
        QVariantMap map;
        argument >> map;
        
        QVariantMap resultMap;
        for (auto iter = map.begin(); iter != map.end(); ++iter) {
            resultMap[iter.key()] = readDBusValue(iter.value());
        }
        return resultMap;
    }
    
    if (value.type() == QVariant::Map) {
        QVariantMap resultMap;
        const auto map = value.toMap();
        for (auto iter = map.begin(); iter != map.end(); ++iter) {
            resultMap[iter.key()] = readDBusValue(iter.value());
        }
        return resultMap;
    }
    
    return value;
}

Fcitx5ConfigProxy::Fcitx5ConfigProxy(fcitx::kcm::DBusProvider *dbus, const QString &path, QObject *parent)
    : QObject(parent), d(new Fcitx5ConfigProxyPrivate(this, dbus, path))
{
}

Fcitx5ConfigProxy::~Fcitx5ConfigProxy() = default;

QVariantList Fcitx5ConfigProxy::globalConfigTypes() const
{
    QVariantList list;
    for (const auto &type : d->configTypes) {
        if (type.name() == "GlobalConfig") {
            for (const auto &option : type.options()) {
                QVariantMap item;
                item["name"] = option.name();
                item["description"] = option.description();
                list.append(item);
            }
            break;
        }
    }
    return list;
}

QVariantList Fcitx5ConfigProxy::globalConfigOptions(const QString &type) const
{
    QVariantList list;
    QString currentType = type+"$"+type+"Config";
    for (const auto &configType : d->configTypes) {
        if (configType.name() != currentType)
            continue;
        for (const auto &option : configType.options()) {
            QVariantMap item;
            item["name"] = option.name();
            item["type"] = option.type();
            item["description"] = option.description();
            auto variant = value(type+"/"+option.name());
            if (variant.type() == QVariant::Map) {
                QVariantMap map = variant.toMap();
                if (map.contains("0")) {
                    item["value"] = d->formatKey(map["0"].toString());
                }
            } else {
                item["value"] = variant;
            }
            
            QVariantMap properties = option.properties();
            if (!properties.isEmpty()) {
                auto iterator = properties.begin();
                while (iterator != properties.constEnd()) {
                    if (iterator.key() == "Enum") {
                        auto argument = qvariant_cast<QDBusArgument>(iterator.value());
                        QVariantMap map;
                        argument >> map;
                        QVariantList enumStrings = map.values().toList();
                        if (!enumStrings.isEmpty()) {
                            item["properties"] = enumStrings;
                        }
                        break;
                    }
                    ++iterator;
                }
                iterator = properties.begin();
                while (iterator != properties.constEnd()) {
                    if (iterator.key() == "EnumI18n") {
                        auto argument = qvariant_cast<QDBusArgument>(iterator.value());
                        QVariantMap map;
                        argument >> map;
                        QVariantList enumStrings = map.values().toList();
                        if (!enumStrings.isEmpty()) {
                            item["propertiesI18n"] = enumStrings;
                        }
                        break;
                    }
                    ++iterator;
                }
            }
            list.append(item);
        }
        break;
    }
    return list;
}

void Fcitx5ConfigProxy::requestConfig(bool sync)
{
    if (!d->dbusprovider->controller()) {
        return;
    }
    auto call = d->dbusprovider->controller()->GetConfig(d->path);
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher,
            &QDBusPendingCallWatcher::finished,
            this,
            &Fcitx5ConfigProxy::onRequestConfigFinished);
    if (sync) {
        watcher->waitForFinished();
    }
}
 
void Fcitx5ConfigProxy::onRequestConfigFinished(QDBusPendingCallWatcher *watcher)
{
    watcher->deleteLater();
    QDBusPendingReply<QDBusVariant, fcitx::FcitxQtConfigTypeList> reply = *watcher;
    if (reply.isError()) {
        qWarning() << reply.error();
        return;
    }
    d->configTypes = reply.argumentAt<1>(); 

    auto value = reply.argumentAt<0>().variant();
    QVariantMap allMap;
    allMap = d->readDBusValue(value).toMap();
    std::swap(d->configValue, allMap);

    Q_EMIT requestConfigFinished();
}

QVariant Fcitx5ConfigProxy::value(const QString &path) const
{
    return fcitx::kcm::readVariant(d->configValue, path);
}

void Fcitx5ConfigProxy::setValue(const QString &path, const QVariant &value, bool isKey)
{
    if (value == this->value(path))
        return;
    if (isKey) {
        auto keys = d->formatKey(value.toString());
        fcitx::kcm::writeVariant(d->configValue, path + "/0", keys);
    } else {
        fcitx::kcm::writeVariant(d->configValue, path, value);
    }
    d->timer->start();
}

void Fcitx5ConfigProxy::save()
{
    if (!d->dbusprovider->controller()) {
        return;
    }

    QDBusVariant var(d->configValue);
    d->dbusprovider->controller()->SetConfig(d->path, var);
    requestConfig(false);
}
