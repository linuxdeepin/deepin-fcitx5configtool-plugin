// SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#include "glo.h"
#include "model.h"
#include "dbusprovider.h"
#include "imconfig.h"

#include <QDebug>

IMConfig::IMConfig(DBusProvider *dbus, ModelMode mode, QObject *parent)
    : QObject(parent), m_dbus(dbus), m_availIMModel(new IMProxyModel(this)),
      m_currentIMModel(new FilteredIMModel(FilteredIMModel::CurrentIM, this)) {
    connect(dbus, &DBusProvider::availabilityChanged, this,
            &IMConfig::availabilityChanged);
    availabilityChanged();

    if (mode == Flatten) {
        auto flattenAvailIMModel =
            new FilteredIMModel(FilteredIMModel::AvailIM, this);
        m_availIMModel->setSourceModel(flattenAvailIMModel);
        m_internalAvailIMModel = flattenAvailIMModel;
    } else {
        auto availIMModel = new AvailIMModel(this);
        m_availIMModel->setSourceModel(availIMModel);
        m_internalAvailIMModel = availIMModel;
    }

    connect(m_currentIMModel, &FilteredIMModel::imListChanged, this,
            [this](const FcitxQtInputMethodEntryList &list) {
                auto old = m_imEntries;
                FcitxQtStringKeyValueList newEntries;
                for (const auto &item : list) {
                    auto iter = std::find_if(
                        old.begin(), old.end(),
                        [&item](const FcitxQtStringKeyValue &entry) {
                            return entry.key() == item.uniqueName();
                        });
                    if (iter != old.end()) {
                        newEntries.push_back(*iter);
                    }
                }
                m_imEntries = newEntries;
                updateIMList(true);
                emitChanged();
            });
}

IMConfig::~IMConfig() {}

void IMConfig::save() {
    if (!m_dbus->controller()) {
        return;
    }
    if (m_needSave) {
        m_dbus->controller()->SetInputMethodGroupInfo(m_lastGroup, m_defaultLayout,
                                                     m_imEntries);
        m_needSave = false;
    }
}

void IMConfig::load() { availabilityChanged(); }

void IMConfig::defaults() {}

void IMConfig::addIM(int idx) {
    auto index = m_availIMModel->index(idx, 0);
    addIM(index);
}

void IMConfig::addIM(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }
    auto uniqueName = index.data(FcitxIMUniqueNameRole).toString();
    FcitxQtStringKeyValue imEntry;
    imEntry.setKey(uniqueName);
    m_imEntries.push_back(imEntry);
    updateIMList();
    emitChanged();
}

void IMConfig::addIMs(const QModelIndexList &indexes) {
    for (const auto &index : indexes) {
        if (!index.isValid()) {
            continue;
        }
        auto uniqueName = index.data(FcitxIMUniqueNameRole).toString();
        FcitxQtStringKeyValue imEntry;
        imEntry.setKey(uniqueName);
        m_imEntries.push_back(imEntry);
    }
    updateIMList();
    emitChanged();
}

void IMConfig::removeIM(int idx) { m_currentIMModel->remove(idx); }

void IMConfig::removeIM(const QModelIndex &index) {
    m_currentIMModel->remove(index.row());
}

void IMConfig::move(int from, int to) { m_currentIMModel->move(from, to); }

void IMConfig::reloadGroup() {
    if (!m_dbus->controller()) {
        return;
    }
    auto call = m_dbus->controller()->InputMethodGroups();
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *watcher) {
                fetchGroupsFinished(watcher);
            });
}

void IMConfig::fetchGroupsFinished(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<QStringList> groups = *watcher;
    watcher->deleteLater();

    if (groups.isValid()) {
        m_groups = groups.value();
        Q_EMIT groupsChanged(m_groups);
    }

    if (!m_groups.empty()) {
        setCurrentGroup(m_groups.front());
    }
}

void IMConfig::availabilityChanged() {
    m_lastGroup.clear();
    if (!m_dbus->controller()) {
        return;
    }
    reloadGroup();
    auto imcall = m_dbus->controller()->AvailableInputMethods();
    auto imcallwatcher = new QDBusPendingCallWatcher(imcall, this);
    connect(imcallwatcher, &QDBusPendingCallWatcher::finished, this,
            &IMConfig::fetchInputMethodsFinished);
    auto checkUpdate = m_dbus->controller()->CheckUpdate();
    auto checkUpdateWatcher = new QDBusPendingCallWatcher(checkUpdate, this);
    connect(checkUpdateWatcher, &QDBusPendingCallWatcher::finished, this,
            &IMConfig::checkUpdateFinished);
}

void IMConfig::fetchInputMethodsFinished(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<FcitxQtInputMethodEntryList> ims = *watcher;
    watcher->deleteLater();
    if (!ims.isError()) {
        m_allIMs = ims.value();
        updateIMList();
    }
}

void IMConfig::checkUpdateFinished(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<bool> reply = *watcher;
    watcher->deleteLater();
    const bool needUpdate = !reply.isError() && reply.value();
    if (m_needUpdate != needUpdate) {
        m_needUpdate = needUpdate;
        Q_EMIT needUpdateChanged(m_needUpdate);
    }
}

void IMConfig::setCurrentGroup(const QString &name) {
    if (m_dbus->available() && !name.isEmpty()) {
        auto call = m_dbus->controller()->InputMethodGroupInfo(name);
        m_lastGroup = name;
        Q_EMIT currentGroupChanged(m_lastGroup);
        auto watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                &IMConfig::fetchGroupInfoFinished);
    }
}

void IMConfig::fetchGroupInfoFinished(QDBusPendingCallWatcher *watcher) {
    watcher->deleteLater();
    m_needSave = false;
    QDBusPendingReply<QString, FcitxQtStringKeyValueList> reply = *watcher;
    if (!reply.isError()) {
        m_defaultLayout = reply.argumentAt<0>();
        m_imEntries = reply.argumentAt<1>();
    } else {
        m_defaultLayout.clear();
        m_imEntries.clear();
    }
    Q_EMIT defaultLayoutChanged();

    updateIMList();
}

void IMConfig::emitChanged() {
    m_needSave = true;
    Q_EMIT changed();
}

void IMConfig::updateIMList(bool excludeCurrent) {
    if (!excludeCurrent) {
        m_currentIMModel->filterIMEntryList(m_allIMs, m_imEntries);
    }
    m_internalAvailIMModel->filterIMEntryList(m_allIMs, m_imEntries);
    m_availIMModel->filterIMEntryList(m_allIMs, m_imEntries);

    Q_EMIT imListChanged();
}

void IMConfig::addGroup(const QString &name) {
    if (!name.isEmpty() && m_dbus->controller()) {
        auto call = m_dbus->controller()->AddInputMethodGroup(name);
        auto watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                [this](QDBusPendingCallWatcher *watcher) {
                    watcher->deleteLater();
                    if (!watcher->isError()) {
                        reloadGroup();
                    }
                });
    }
}

void IMConfig::deleteGroup(const QString &name) {
    if (!m_dbus->controller()) {
        return;
    }
    auto call = m_dbus->controller()->RemoveInputMethodGroup(name);
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *watcher) {
                watcher->deleteLater();
                if (!watcher->isError()) {
                    reloadGroup();
                }
            });
}

void IMConfig::refresh() {
    if (!m_dbus->controller()) {
        return;
    }
    auto call = m_dbus->controller()->Refresh();
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *watcher) {
                watcher->deleteLater();
                if (!watcher->isError()) {
                    load();
                }
            });
}

void IMConfig::restart() {
    if (!m_dbus->controller()) {
        return;
    }
    m_dbus->controller()->Restart();
}
