/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "imconfig.h"
#include "dbusprovider.h"
#include "model.h"

IMConfig::IMConfig(DBusProvider *dbus, ModelMode mode, QObject *parent)
    : QObject(parent), dbus_(dbus), availIMModel_(new IMProxyModel(this)),
      m_currentInputMethodList(new FcitxQtInputMethodItemList) {
    connect(dbus, &DBusProvider::availabilityChanged, this,
            &IMConfig::availabilityChanged);
    availabilityChanged();

    if (mode == Flatten) {
        auto flattenAvailIMModel =
            new FilteredIMModel(FilteredIMModel::AvailIM, this);
        availIMModel_->setSourceModel(flattenAvailIMModel);
        internalAvailIMModel_ = flattenAvailIMModel;
    } else {
        auto availIMModel = new AvailIMModel(this);
        availIMModel_->setSourceModel(availIMModel);
        internalAvailIMModel_ = availIMModel;
    }
}

IMConfig::~IMConfig() {}

void IMConfig::save() {
    if (!dbus_->controller()) {
        return;
    }
    if (needSave_) {
        dbus_->controller()->SetInputMethodGroupInfo(lastGroup_, defaultLayout_,
                                                     imEntries_);
        needSave_ = false;
    }
}

void IMConfig::load() { availabilityChanged(); }

void IMConfig::defaults() {}

//void IMConfig::addIM(int idx) {
//    auto index = availIMModel_->index(idx, 0);
//    addIM(index);
//}

void IMConfig::addIM(FcitxQtInputMethodItem* item)
{
    m_currentInputMethodList->append(item);
    reloadIMList();
    save();
}

//void IMConfig::removeIM(int idx) { m_currentInputMethodList->removeAt(idx); }

void IMConfig::removeIM(int index) {
    m_currentInputMethodList->removeAt(index);
    reloadIMList();
    save();
}

void IMConfig::move(int from, int to) {
    m_currentInputMethodList->move(from, to);
    reloadIMList();
    save();
}

void IMConfig::reloadGroup() {
    if (!dbus_->controller()) {
        return;
    }
    auto call = dbus_->controller()->InputMethodGroups();
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
        groups_ = groups.value();
        emit groupsChanged(groups_);
    }

    if (!groups_.empty()) {
        setCurrentGroup(groups_.front());
    }
}

void IMConfig::reloadIMList()
{
    auto old = imEntries_;
    FcitxQtStringKeyValueList newEntries;
    for (int i = 0; i < m_currentInputMethodList->count(); ++i) {
        auto item = m_currentInputMethodList->at(i);
        auto iter = std::find_if(
            old.begin(), old.end(),
            [&item](const FcitxQtStringKeyValue &entry) {
                return entry.key() == item->uniqueName();
            });
        if (iter != old.end()) {
            newEntries.push_back(*iter);
        }
    }
    imEntries_ = newEntries;
    updateIMList(true);
    emitChanged();
}

void IMConfig::availabilityChanged() {
    lastGroup_.clear();
    if (!dbus_->controller()) {
        return;
    }
    reloadGroup();
    auto imcall = dbus_->controller()->AvailableInputMethods();
    auto imcallwatcher = new QDBusPendingCallWatcher(imcall, this);
    connect(imcallwatcher, &QDBusPendingCallWatcher::finished, this,
            &IMConfig::fetchInputMethodsFinished);
}

void IMConfig::fetchInputMethodsFinished(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<FcitxQtInputMethodEntryList> ims = *watcher;
    watcher->deleteLater();
    if (!ims.isError()) {
        allIMs_ = ims.value();
        updateIMList();
    }
    return;
}

void IMConfig::setCurrentGroup(const QString &name) {
    if (dbus_->available() && !name.isEmpty()) {
        auto call = dbus_->controller()->InputMethodGroupInfo(name);
        lastGroup_ = name;
        emit currentGroupChanged(lastGroup_);
        auto watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                &IMConfig::fetchGroupInfoFinished);
    }
}

void IMConfig::fetchGroupInfoFinished(QDBusPendingCallWatcher *watcher) {
    watcher->deleteLater();
    needSave_ = false;
    QDBusPendingReply<QString, FcitxQtStringKeyValueList> reply = *watcher;
    if (!reply.isError()) {
        defaultLayout_ = reply.argumentAt<0>();
        imEntries_ = reply.argumentAt<1>();
    } else {
        defaultLayout_.clear();
        imEntries_.clear();
    }
    emit defaultLayoutChanged();

    updateIMList();
}

void IMConfig::emitChanged() {
    needSave_ = true;
    emit changed();
}

void IMConfig::updateIMList(bool excludeCurrent) {
    if (!excludeCurrent) {
        filterIMEntryList(allIMs_, imEntries_);
    }
    internalAvailIMModel_->filterIMEntryList(allIMs_, imEntries_);
    availIMModel_->filterIMEntryList(allIMs_, imEntries_);

    emit imListChanged();
}

void IMConfig::filterIMEntryList(
    const FcitxQtInputMethodEntryList &imEntryList,
    const FcitxQtStringKeyValueList &enabledIMList) {

    m_currentInputMethodList->clear();
    //enabledIMList_ = enabledIMList;

    // We implement this twice for following reasons:
    // 1. "enabledIMs" is usually very small.
    // 2. CurrentIM mode need to keep order by enabledIMs.
    if (1/*mode_ == CurrentIM*/) {
        int row = 0;
        QMap<QString, const FcitxQtInputMethodEntry *> nameMap;
        for (auto &imEntry : imEntryList) {
            nameMap.insert(imEntry.uniqueName(), &imEntry);
        }

        for (const auto &im : enabledIMList) {
            if (auto value = nameMap.value(im.key(), nullptr)) {
                FcitxQtInputMethodItem *item = new  FcitxQtInputMethodItem;
                item->setName(value->name());
                item->setUniqueName(value->uniqueName());
                item->setConfigurable(value->configurable());
                item->setLanguageCode(value->languageCode());
                m_currentInputMethodList->append(item);
                row++;
            }
        }
    } else if (0/*mode_ == AvailIM*/) {
        QSet<QString> enabledIMs;
        for (const auto &item : enabledIMList) {
            enabledIMs.insert(item.key());
        }

        for (const FcitxQtInputMethodEntry &im : imEntryList) {
            if (enabledIMs.contains(im.uniqueName())) {
                continue;
            }
            FcitxQtInputMethodItem *item = new  FcitxQtInputMethodItem;
            item->setName(im.name());
            item->setUniqueName(im.uniqueName());
            item->setConfigurable(im.configurable());
            item->setLanguageCode(im.languageCode());
            m_currentInputMethodList->append(item);
        }
    }
}

void IMConfig::addGroup(const QString &name) {
    if (!name.isEmpty() && dbus_->controller()) {
        auto call = dbus_->controller()->AddInputMethodGroup(name);
        auto watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
            watcher->deleteLater();
            if (!watcher->isError()) {
                reloadGroup();
            }
        });
    }
}

void IMConfig::deleteGroup(const QString &name) {
    if (dbus_->controller()) {
        auto call = dbus_->controller()->RemoveInputMethodGroup(name);
        auto watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
            watcher->deleteLater();
            if (!watcher->isError()) {
                reloadGroup();
            }
        });
    }
}
