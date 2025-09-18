/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "imconfig.h"
#include "dbusprovider.h"
#include "model.h"
#include "logging.h"

namespace fcitx {
namespace kcm {

IMConfig::IMConfig(DBusProvider *dbus, ModelMode mode, QObject *parent)
    : QObject(parent), dbus_(dbus), availIMModel_(new IMProxyModel(this)),
      currentIMModel_(new FilteredIMModel(FilteredIMModel::CurrentIM, this)) {
    qCDebug(KCM_FCITX5) << "Initializing IMConfig with mode:" << mode;
    connect(dbus, &DBusProvider::availabilityChanged, this,
            &IMConfig::availabilityChanged);
    availabilityChanged();

    if (mode == Flatten) {
        qCDebug(KCM_FCITX5) << "Setting up flatten mode";
        auto flattenAvailIMModel =
            new FilteredIMModel(FilteredIMModel::AvailIM, this);
        availIMModel_->setSourceModel(flattenAvailIMModel);
        internalAvailIMModel_ = flattenAvailIMModel;
    } else {
        qCDebug(KCM_FCITX5) << "Setting up categorized mode";
        auto availIMModel = new AvailIMModel(this);
        availIMModel_->setSourceModel(availIMModel);
        internalAvailIMModel_ = availIMModel;
    }

    connect(currentIMModel_, &FilteredIMModel::imListChanged, this,
            [this](const FcitxQtInputMethodEntryList &list) {
                auto old = imEntries_;
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
                imEntries_ = newEntries;
                updateIMList(true);
                emitChanged();
            });
    qCDebug(KCM_FCITX5) << "Exiting IMConfig constructor";
}

IMConfig::~IMConfig() {
    qCDebug(KCM_FCITX5) << "Destroying IMConfig";
}

void IMConfig::save() {
    qCDebug(KCM_FCITX5) << "Entering save";
    if (!dbus_->controller()) {
        qCWarning(KCM_FCITX5) << "Cannot save - DBus controller not available";
        return;
    }
    if (needSave_) {
        qCInfo(KCM_FCITX5) << "Saving IM configuration for group:" << lastGroup_
                         << "with" << imEntries_.size() << "IM entries";
        dbus_->controller()->SetInputMethodGroupInfo(lastGroup_, defaultLayout_,
                                                     imEntries_);
        needSave_ = false;
    } else {
        qCDebug(KCM_FCITX5) << "No changes to save";
    }
    qCDebug(KCM_FCITX5) << "Exiting save";
}

void IMConfig::load() {
    qCDebug(KCM_FCITX5) << "Loading IM configuration";
    availabilityChanged();
}

void IMConfig::defaults() {
    qCDebug(KCM_FCITX5) << "Resetting IM config to defaults";
}

void IMConfig::addIM(int idx) {
    qCDebug(KCM_FCITX5) << "Entering addIM with index:" << idx;
    auto index = availIMModel_->index(idx, 0);
    addIM(index);
    qCDebug(KCM_FCITX5) << "Exiting addIM";
}

void IMConfig::addIM(const QModelIndex &index) {
    qCDebug(KCM_FCITX5) << "Entering addIM with model index";
    if (!index.isValid()) {
        qCWarning(KCM_FCITX5) << "Attempted to add IM with invalid index";
        return;
    }
    auto uniqueName = index.data(FcitxIMUniqueNameRole).toString();
    qCInfo(KCM_FCITX5) << "Adding IM:" << uniqueName;
    FcitxQtStringKeyValue imEntry;
    imEntry.setKey(uniqueName);
    imEntries_.push_back(imEntry);
    updateIMList();
    emitChanged();
    qCDebug(KCM_FCITX5) << "Exiting addIM";
}

void IMConfig::addIMs(const QModelIndexList &indexes) {
    qCDebug(KCM_FCITX5) << "Entering addIMs with" << indexes.count() << "indexes.";
    for (const auto &index : indexes) {
        if (!index.isValid()) {
            // qCWarning(KCM_FCITX5) << "Skipping invalid index in addIMs";
            continue;
        }
        auto uniqueName = index.data(FcitxIMUniqueNameRole).toString();
        FcitxQtStringKeyValue imEntry;
        imEntry.setKey(uniqueName);
        imEntries_.push_back(imEntry);
    }
    updateIMList();
    emitChanged();
    qCDebug(KCM_FCITX5) << "Exiting addIMs";
}

void IMConfig::removeIM(int idx) {
    qCInfo(KCM_FCITX5) << "Removing IM at index:" << idx;
    currentIMModel_->remove(idx);
}

void IMConfig::removeIM(const QModelIndex &index) {
    qCInfo(KCM_FCITX5) << "Removing IM at row:" << index.row();
    currentIMModel_->remove(index.row());
}

void IMConfig::move(int from, int to) {
    qCInfo(KCM_FCITX5) << "Moving IM from" << from << "to" << to;
    currentIMModel_->move(from, to);
}

void IMConfig::reloadGroup() {
    qCInfo(KCM_FCITX5) << "Reloading input method groups.";
    if (!dbus_->controller()) {
        qCWarning(KCM_FCITX5) << "Cannot reload groups, DBus controller not available.";
        return;
    }
    auto call = dbus_->controller()->InputMethodGroups();
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *watcher) {
                fetchGroupsFinished(watcher);
            });
    qCDebug(KCM_FCITX5) << "Reloading input method groups";
}

void IMConfig::fetchGroupsFinished(QDBusPendingCallWatcher *watcher) {
    qCDebug(KCM_FCITX5) << "Processing fetched input method groups";
    QDBusPendingReply<QStringList> groups = *watcher;
    watcher->deleteLater();

    if (groups.isValid()) {
        groups_ = groups.value();
        qCInfo(KCM_FCITX5) << "Fetched" << groups_.count() << "input method groups.";
        Q_EMIT groupsChanged(groups_);
    }

    if (!groups_.empty()) {
        qCDebug(KCM_FCITX5) << "Setting current group to:" << groups_.front();
        setCurrentGroup(groups_.front());
    }
    qCDebug(KCM_FCITX5) << "Exiting fetchGroupsFinished";
}

void IMConfig::availabilityChanged() {
    qCDebug(KCM_FCITX5) << "DBus availability changed, controller available:"
                     << (dbus_->controller() != nullptr);
    lastGroup_.clear();
    if (!dbus_->controller()) {
        qCWarning(KCM_FCITX5) << "DBus controller not available, skipping operations";
        return;
    }
    // qCDebug(KCM_FCITX5) << "Reloading input method groups";
    reloadGroup();
    // qCDebug(KCM_FCITX5) << "Fetching available input methods";
    auto imcall = dbus_->controller()->AvailableInputMethods();
    auto imcallwatcher = new QDBusPendingCallWatcher(imcall, this);
    connect(imcallwatcher, &QDBusPendingCallWatcher::finished, this,
            &IMConfig::fetchInputMethodsFinished);
    // qCDebug(KCM_FCITX5) << "Checking for updates";
    auto checkUpdate = dbus_->controller()->CheckUpdate();
    auto checkUpdateWatcher = new QDBusPendingCallWatcher(checkUpdate, this);
    connect(checkUpdateWatcher, &QDBusPendingCallWatcher::finished, this,
            &IMConfig::checkUpdateFinished);
    // qCDebug(KCM_FCITX5) << "Exiting availabilityChanged";
}

void IMConfig::fetchInputMethodsFinished(QDBusPendingCallWatcher *watcher) {
    qCDebug(KCM_FCITX5) << "Processing fetched input methods";
    // qCDebug(KCM_FCITX5) << "DBus call error:" << watcher->error().message();
    QDBusPendingReply<FcitxQtInputMethodEntryList> ims = *watcher;
    watcher->deleteLater();
    if (!ims.isError()) {
        qCDebug(KCM_FCITX5) << "Fetched" << ims.value().count() << "available input methods";
        allIMs_ = ims.value();
        updateIMList();
    }
    qCDebug(KCM_FCITX5) << "Exiting fetchInputMethodsFinished";
}

void IMConfig::checkUpdateFinished(QDBusPendingCallWatcher *watcher) {
    qCDebug(KCM_FCITX5) << "Entering checkUpdateFinished";
    QDBusPendingReply<bool> reply = *watcher;
    watcher->deleteLater();
    const bool needUpdate = !reply.isError() && reply.value();
    if (needUpdate_ != needUpdate) {
        qCDebug(KCM_FCITX5) << "Update check finished, needUpdate status:" << needUpdate;
        needUpdate_ = needUpdate;
        Q_EMIT needUpdateChanged(needUpdate_);
    }
    qCDebug(KCM_FCITX5) << "Exiting checkUpdateFinished";
}

void IMConfig::setCurrentGroup(const QString &name) {
    qCDebug(KCM_FCITX5) << "Setting current group to:" << name;
    if (dbus_->available() && !name.isEmpty()) {
        qCDebug(KCM_FCITX5) << "Fetching group info for:" << name;
        auto call = dbus_->controller()->InputMethodGroupInfo(name);
        lastGroup_ = name;
        Q_EMIT currentGroupChanged(lastGroup_);
        auto watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                &IMConfig::fetchGroupInfoFinished);
    }
    qCDebug(KCM_FCITX5) << "Exiting setCurrentGroup";
}

void IMConfig::fetchGroupInfoFinished(QDBusPendingCallWatcher *watcher) {
    qCDebug(KCM_FCITX5) << "Processing group info for:" << lastGroup_;
    // qCDebug(KCM_FCITX5) << "DBus call error:" << watcher->error().message();
    watcher->deleteLater();
    needSave_ = false;
    QDBusPendingReply<QString, FcitxQtStringKeyValueList> reply = *watcher;
    if (!reply.isError()) {
        qCDebug(KCM_FCITX5) << "Fetched group info for:" << lastGroup_;
        defaultLayout_ = reply.argumentAt<0>();
        imEntries_ = reply.argumentAt<1>();
    } else {
        qCDebug(KCM_FCITX5) << "Failed to fetch group info for:" << lastGroup_;
        defaultLayout_.clear();
        imEntries_.clear();
    }
    Q_EMIT defaultLayoutChanged();

    updateIMList();
    qCDebug(KCM_FCITX5) << "Exiting fetchGroupInfoFinished";
}

void IMConfig::emitChanged() {
    qCDebug(KCM_FCITX5) << "Emitting changed signal, needSave set to true.";
    needSave_ = true;
    Q_EMIT changed();
}

void IMConfig::updateIMList(bool excludeCurrent) {
    qCDebug(KCM_FCITX5) << "Updating IM list, excludeCurrent:" << excludeCurrent;
    // qCDebug(KCM_FCITX5) << "Current IM entries count:" << imEntries_.size();
    if (!excludeCurrent) {
        qCDebug(KCM_FCITX5) << "Updating currentIMModel with" << allIMs_.count() << "total IMs and" << imEntries_.count() << "current entries.";
        currentIMModel_->filterIMEntryList(allIMs_, imEntries_);
    }
    internalAvailIMModel_->filterIMEntryList(allIMs_, imEntries_);
    availIMModel_->filterIMEntryList(allIMs_, imEntries_);

    Q_EMIT imListChanged();
    qCDebug(KCM_FCITX5) << "Exiting updateIMList";
}

void IMConfig::addGroup(const QString &name) {
    qCDebug(KCM_FCITX5) << "Adding new input method group:" << name;
    if (!name.isEmpty() && dbus_->controller()) {
        qCDebug(KCM_FCITX5) << "Adding new input method group:" << name;
        auto call = dbus_->controller()->AddInputMethodGroup(name);
        auto watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                [this](QDBusPendingCallWatcher *watcher) {
                    watcher->deleteLater();
                    if (!watcher->isError()) {
                        reloadGroup();
                    }
                });
    }
    qCDebug(KCM_FCITX5) << "Exiting addGroup";
}

void IMConfig::deleteGroup(const QString &name) {
    qCDebug(KCM_FCITX5) << "Deleting input method group:" << name;
    if (!dbus_->controller()) {
        qCWarning(KCM_FCITX5) << "Cannot delete group, DBus controller not available.";
        return;
    }
    auto call = dbus_->controller()->RemoveInputMethodGroup(name);
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *watcher) {
                watcher->deleteLater();
                if (!watcher->isError()) {
                    qCDebug(KCM_FCITX5) << "Group deleted successfully, reloading groups";
                    reloadGroup();
                }
            });
    qCDebug(KCM_FCITX5) << "Exiting deleteGroup";
}

void IMConfig::refresh() {
    qCDebug(KCM_FCITX5) << "Refreshing input method configuration";
    if (!dbus_->controller()) {
        qCWarning(KCM_FCITX5) << "Cannot refresh, DBus controller not available.";
        return;
    }
    auto call = dbus_->controller()->Refresh();
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *watcher) {
                watcher->deleteLater();
                if (!watcher->isError()) {
                    qCDebug(KCM_FCITX5) << "Refresh successful, reloading configuration";
                    load();
                }
            });
    qCDebug(KCM_FCITX5) << "Exiting refresh";
}

void IMConfig::restart() {
    qCDebug(KCM_FCITX5) << "Restarting input method service";
    if (!dbus_->controller()) {
        qCWarning(KCM_FCITX5) << "Cannot restart, DBus controller not available.";
        return;
    }
    dbus_->controller()->Restart();
    qCDebug(KCM_FCITX5) << "Exiting restart";
}

} // namespace kcm
} // namespace fcitx
