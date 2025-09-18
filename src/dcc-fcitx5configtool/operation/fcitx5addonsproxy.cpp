// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fcitx5addonsproxy.h"
#include "private/fcitx5addonsproxy_p.h"

#include <dbusprovider.h>
#include <varianthelper.h>

#include <fcitx-utils/standardpath.h>

#include <QDBusPendingCallWatcher>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(addonsProxy, "fcitx5.configtool.addonsproxy")

using namespace deepin::fcitx5configtool;

Fcitx5AddonsProxyPrivate::Fcitx5AddonsProxyPrivate(Fcitx5AddonsProxy *parent, fcitx::kcm::DBusProvider *dbus)
    : q(parent), dbusprovider(dbus)
{
    qCDebug(addonsProxy) << "Entering Fcitx5AddonsProxyPrivate constructor";
}

void Fcitx5AddonsProxyPrivate::fetchAddonsFinished(QDBusPendingCallWatcher *watcher)
{
    qCDebug(addonsProxy) << "Entering fetchAddonsFinished";
    watcher->deleteLater();

    if (watcher->isError()) {
        qCWarning(addonsProxy) << "Failed to fetch addons:" << watcher->error().message();
        return;
    }
    QDBusPendingReply<fcitx::FcitxQtAddonInfoV2List> reply(*watcher);
    nameToAddonMap.clear();
    reverseDependencies.clear();
    reverseOptionalDependencies.clear();
    auto list = reply.value();
    for (const auto &addon : list) {
        nameToAddonMap[addon.uniqueName()] = addon;
    }

    addonEntryList.clear();
    QMap<int, int> addonCategoryMap;
    for (const auto &addon : list) {
        for (const auto &dep : addon.dependencies()) {
            if (!nameToAddonMap.contains(dep)) {
                continue;
            }
            reverseDependencies[dep].append(addon.uniqueName());
        }
        for (const auto &dep : addon.optionalDependencies()) {
            if (!nameToAddonMap.contains(dep)) {
                continue;
            }
            reverseOptionalDependencies[dep].append(addon.uniqueName());
        }

        int idx;
        if (!addonCategoryMap.contains(addon.category())) {
            idx = addonEntryList.count();
            addonCategoryMap[addon.category()] = idx;
            addonEntryList.append(QPair<int, QStringList>(addon.category(), QStringList()));
        } else {
            idx = addonCategoryMap[addon.category()];
        }
        addonEntryList[idx].second.append(addon.uniqueName());
    }

    Q_EMIT q->requestAddonsFinished();
    qCDebug(addonsProxy) << "Exiting fetchAddonsFinished";
}

Fcitx5AddonsProxy::Fcitx5AddonsProxy(fcitx::kcm::DBusProvider *dbus, QObject *parent)
    : QObject(parent), d(new Fcitx5AddonsProxyPrivate(this, dbus))
{
    qCDebug(addonsProxy) << "Entering Fcitx5AddonsProxy constructor";
}

Fcitx5AddonsProxy::~Fcitx5AddonsProxy() = default;

void Fcitx5AddonsProxy::clear()
{
    qCDebug(addonsProxy) << "Entering clear";
    d->nameToAddonMap.clear();
    d->reverseDependencies.clear();
    d->reverseOptionalDependencies.clear();
    d->addonEntryList.clear();
    qCDebug(addonsProxy) << "Exiting clear";
}

void Fcitx5AddonsProxy::load()
{
    qCDebug(addonsProxy) << "Entering load";
    if (!d->dbusprovider->controller()) {
        qCWarning(addonsProxy) << "DBus controller not available for load";
        return;
    }

    auto call = d->dbusprovider->controller()->GetAddonsV2();
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, d,
            &Fcitx5AddonsProxyPrivate::fetchAddonsFinished);
    qCDebug(addonsProxy) << "Exiting load";
}

QVariantList Fcitx5AddonsProxy::globalAddons() const
{
    qCDebug(addonsProxy) << "Entering globalAddons";
    QVariantList ret;
    for (const auto &entry : d->addonEntryList) {
        for (const auto &addonUniqueName : entry.second) {
            const auto &addon = d->nameToAddonMap[addonUniqueName];
            QVariantMap addonMap;
            addonMap["name"] = addon.name();
            addonMap["uniqueName"] = addon.uniqueName();
            addonMap["comment"] = addon.comment();
            addonMap["category"] = addon.category();
            addonMap["configurable"] = addon.configurable();
            addonMap["enabled"] = addon.enabled();
            addonMap["onDemand"] = addon.onDemand();
            addonMap["dependencies"] = addon.dependencies();
            addonMap["optionalDependencies"] = addon.optionalDependencies();
            ret.append(QVariant::fromValue(addonMap));
        }
    }
    qCDebug(addonsProxy) << "Exiting globalAddons with" << ret.size() << "items";
    return ret;
}

void Fcitx5AddonsProxy::setEnableAddon(const QString &addonStr, bool enable)
{
    qCDebug(addonsProxy) << "Entering setEnableAddon for addon" << addonStr << "with enable state" << enable;
    if (!d->nameToAddonMap.contains(addonStr)) {
        qCWarning(addonsProxy) << "Addon not found:" << addonStr;
        return;
    }
    
    fcitx::FcitxQtAddonStateList list;
    auto &addon = d->nameToAddonMap[addonStr];
    addon.setEnabled(enable);
    fcitx::FcitxQtAddonState state;
    state.setUniqueName(addon.uniqueName());
    state.setEnabled(enable);
    list.append(state);
                
    if (list.size()) {
        qCInfo(addonsProxy) << "Setting addon states and restarting fcitx5.";
        d->dbusprovider->controller()->SetAddonsState(list);
        QProcess p;
        p.start("killall", QStringList() << "fcitx5");
        p.waitForFinished();
        QProcess::startDetached(QString::fromStdString(fcitx::StandardPath::fcitxPath("bindir", "fcitx5")), QStringList());
    }
    qCDebug(addonsProxy) << "Exiting setEnableAddon";
}
