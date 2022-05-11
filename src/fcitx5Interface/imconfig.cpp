/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "glo.h"
#include "imelog.h"
//#include "osastroper.h"
#include "imconfig.h"
#include "dbusprovider.h"
#include "addim_model.h"

IMConfig::IMConfig(DBusProvider *dbus, ModelMode mode, QObject *parent)
    : QObject(parent), dbus_(dbus), availIMModel_(new IMProxyModel(this)),
      m_currentInputMethodList(new FcitxQtInputMethodItemList)
      , m_currentIMModel(new FilteredIMModel(FilteredIMModel::CurrentIM, this)) {
    connect(dbus, &DBusProvider::availabilityChanged, this, &IMConfig::availabilityChanged);
    //availabilityChanged();

    if (mode == Flatten) {
        auto flattenAvailIMModel = new FilteredIMModel(FilteredIMModel::AvailIM, this);
        availIMModel_->setSourceModel(flattenAvailIMModel);
        internalAvailIMModel_ = flattenAvailIMModel;
    } else {
        auto availIMModel = new AvailIMModel(this);
        availIMModel_->setSourceModel(availIMModel);
        internalAvailIMModel_ = availIMModel;
    }

    m_mode = mode;
    availabilityChanged();

    connect(this, &IMConfig::addIMSignal, this, &IMConfig::testAddIMDeal);
}

IMConfig::~IMConfig() {}

void IMConfig::testAddIMDeal(int imIndex)
{
    osaLogInfo(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "====> imIndex [%d]\n", imIndex);
    osaLogInfo(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "<====\n");
}

void IMConfig::save() {
    if (!dbus_->controller()) {
        return;
    }
    if (needSave_) {
        dbus_->controller()->SetInputMethodGroupInfo(lastGroup_, defaultLayout_, imEntries_);
        needSave_ = false;
    }
}

void IMConfig::saveSelectedIM(int imIndex) {
    osaLogInfo(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "====> imIndex [%d], m_needSave [%d]\n", imIndex, needSave_);

    if (!dbus_->controller()) {
        osaLogInfo(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "<====\n");
        return;
    }

    if (imIndex >= m_currentIMEntries.size()) {
        osaLogError(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "<==== ERROR: imIndex >= m_imEntries.size()\n");
        return;
    }

    if (needSave_) {
        FcitxQtStringKeyValueList useIMList = getUseIMList();
        if (useIMList.count() >= 1) {
            useIMList.insert(1, m_currentIMEntries.at(imIndex));
        }
        else {
            useIMList.insert(0, m_currentIMEntries.at(imIndex));
        }
        dbus_->controller()->SetInputMethodGroupInfo(lastGroup_, defaultLayout_, useIMList);
        needSave_ = false;
       // emit addIMSignal(imIndex);
       // updateIMList();
        addIM(m_currentIMEntries.at(imIndex).key());
    }
    osaLogInfo(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "<====\n");
}

void IMConfig::load() {
    availabilityChanged();
}

void IMConfig::defaults() {}

int IMConfig::addSelectedIM(int index, QString matchStr) {
    auto modelIndex = availIMModel_->index(index, 0);
    int count = addSelectedIM(modelIndex);
    return count;
}

void IMConfig::addIM(const QString &name)
{
    int row = 0;
    QMap<QString, const FcitxQtInputMethodEntry *> nameMap;
    for (auto &imEntry : allIMs_) {
        nameMap.insert(imEntry.uniqueName(), &imEntry);
    }

    if (auto value = nameMap.value(name, nullptr)) {
        FcitxQtInputMethodItem *item = new  FcitxQtInputMethodItem;
        item->setName(value->name());
        item->setUniqueName(value->uniqueName());
        item->setConfigurable(value->configurable());
        item->setLanguageCode(value->languageCode());
        if(m_currentInputMethodList->isEmpty()) {
            m_currentInputMethodList->append(item);
        } else {
            m_currentInputMethodList->insert(1, item);
        }
        row++;
    }
    emit imListChanged();
}

int IMConfig::addSelectedIM(const QModelIndex &index, QString matchStr) {
    int count = 0;
    if (!index.isValid()) {
        return count;
    }

    if (!index.parent().isValid()) {
        if (m_mode == Tree) {
            int row_index = index.data(FcitxRowIndexRole).toInt();
            osaLogInfo(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "row_index [%d]\n", row_index);

            m_currentIMEntries.clear();
            FcitxQtStringKeyValueList useIMList = getUseIMList();
            if (matchStr == "") {
                ((AvailIMModel*)internalAvailIMModel_)->getInputMethodEntryList(row_index, m_currentIMEntries, useIMList);
            }
            else {
                ((AvailIMModel*)internalAvailIMModel_)->getInputMethodEntryList(row_index, m_currentIMEntries, useIMList, matchStr);
            }
            count = m_currentIMEntries.count();
        }
        else {
            osaLogError(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "ERROR: m_mode != Tree. m_mode [%d]\n", m_mode);
        }

        updateIMList();
        emitChanged();
    }
    return count;
}

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
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "====>\n");
    QDBusPendingReply<FcitxQtInputMethodEntryList> ims = *watcher;
    watcher->deleteLater();
    if (!ims.isError()) {
        allIMs_ = ims.value();
        updateIMList();
    }
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "<====\n");
    return;
}

void IMConfig::setCurrentGroup(const QString &name) {
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "====> name [%s]\n", name.toStdString().c_str());
    if (dbus_->available() && !name.isEmpty()) {
        auto call = dbus_->controller()->InputMethodGroupInfo(name);
        lastGroup_ = name;
        emit currentGroupChanged(lastGroup_);
        auto watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                &IMConfig::fetchGroupInfoFinished);
    }
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "<====\n");
}

void IMConfig::fetchGroupInfoFinished(QDBusPendingCallWatcher *watcher) {
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "====>\n");
    watcher->deleteLater();
    needSave_ = false;
    QDBusPendingReply<QString, FcitxQtStringKeyValueList> reply = *watcher;
    if (!reply.isError()) {
        defaultLayout_ = reply.argumentAt<0>();
        imEntries_ = reply.argumentAt<1>();
        m_currentIMEntries = reply.argumentAt<1>();
    } else {
        defaultLayout_.clear();
        imEntries_.clear();
        m_currentIMEntries.clear();
    }
    emit defaultLayoutChanged();
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "----> defaultLayout_ [%s]\n", defaultLayout_.toStdString().c_str());
    for (const auto& item : m_currentIMEntries) {
        osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "----> key [%s], item [%s]\n",
            item.key().toStdString().c_str(), item.value().toStdString().c_str());
    }

    setUseIMList(m_currentIMEntries);

    updateIMList();
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "<====\n");
}

void IMConfig::emitChanged() {
    needSave_ = true;
    emit changed();
}

void IMConfig::updateIMList(bool excludeCurrent) {
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "====> excludeCurrent [%d]\n", excludeCurrent);
    if (!excludeCurrent) {
        filterIMEntryList(allIMs_, imEntries_);
        m_currentIMModel->filterIMEntryList(allIMs_, m_currentIMEntries);
    }
    internalAvailIMModel_->filterIMEntryList(allIMs_, imEntries_);
    availIMModel_->filterIMEntryList(allIMs_, imEntries_);

    emit imListChanged();
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "<====\n");
}

void IMConfig::filterIMEntryList(
    const FcitxQtInputMethodEntryList &imEntryList,
    const FcitxQtStringKeyValueList &enabledIMList)
{

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
    if (dbus_->controller()) {
        auto call = dbus_->controller()->RemoveInputMethodGroup(name);
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
