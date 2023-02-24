// SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#include "glo.h"
//#include "osastroper.h"
#include "addimmodel.h"
#include "dbusprovider.h"
#include "imconfig.h"

#include <QDebug>

IMConfig::IMConfig(DBusProvider *dbus, ModelMode mode, QObject *parent)
    : QObject(parent), m_dbus(dbus), m_availIMModel(new IMProxyModel(this)),
      m_currentInputMethodList(new FcitxQtInputMethodItemList)
      , m_currentIMModel(new FilteredIMModel(FilteredIMModel::CurrentIM, this)) {
    connect(dbus, &DBusProvider::availabilityChanged, this, &IMConfig::availabilityChanged);
    //availabilityChanged();

    if (mode == Flatten) {
        auto flattenAvailIMModel = new FilteredIMModel(FilteredIMModel::AvailIM, this);
        m_availIMModel->setSourceModel(flattenAvailIMModel);
        m_internalAvailIMModel = flattenAvailIMModel;
    } else {
        auto availIMModel = new AvailIMModel(this);
        m_availIMModel->setSourceModel(availIMModel);
        m_internalAvailIMModel = availIMModel;
    }

    m_mode = mode;
    availabilityChanged();

    connect(this, &IMConfig::addIMSignal, this, &IMConfig::testAddIMDeal);
}

IMConfig::~IMConfig() {}

void IMConfig::testAddIMDeal(int imIndex)
{
    qInfo("====> imIndex [%d]", imIndex);
    qInfo("<====");
}

void IMConfig::save() {
    if (!m_dbus->controller()) {
        return;
    }
    if (m_needSave) {
        m_dbus->controller()->SetInputMethodGroupInfo(m_lastGroup, m_defaultLayout, m_imEntries);
        m_needSave = false;
    }
}

void IMConfig::saveSelectedIM(int imIndex) {
    qInfo("====> imIndex [%d], m_needSave [%d]", imIndex, m_needSave);

    if (!m_dbus->controller()) {
        qInfo("<====");
        return;
    }

    if (imIndex >= m_currentIMEntries.size()) {
        qWarning("<==== ERROR: imIndex >= m_imEntries.size()");
        return;
    }

        FcitxQtStringKeyValueList &useIMList = getUseIMList();
        if (useIMList.count() >= 1) {
            useIMList.insert(1, m_currentIMEntries.at(imIndex));
        } else {
            useIMList.insert(0, m_currentIMEntries.at(imIndex));
        }
        m_dbus->controller()->SetInputMethodGroupInfo(m_lastGroup, m_defaultLayout, useIMList);
        //needSave_ = false;
        // emit addIMSignal(imIndex);
        // updateIMList();
        addIM(m_currentIMEntries.at(imIndex).key());
        qInfo("<====");
}

void IMConfig::load() {
    availabilityChanged();
}

void IMConfig::defaults() {}

int IMConfig::addSelectedIM(int index, QString matchStr) {
    auto modelIndex = m_availIMModel->index(index, 0);
    int count = addSelectedIM(modelIndex);
    return count;
}

void IMConfig::addIM(const QString &name)
{
    int row = 0;
    QMap<QString, const FcitxQtInputMethodEntry *> nameMap;
    for (auto &imEntry : m_allIMs) {
        nameMap.insert(imEntry.uniqueName(), &imEntry);
    }

    if (auto value = nameMap.value(name, nullptr)) {
        FcitxQtInputMethodItem *item = new  FcitxQtInputMethodItem;
        item->setName(value->name());
        item->setUniqueName(value->uniqueName());
        item->setConfigurable(value->configurable());
        item->setLanguageCode(value->languageCode());

        QString name = item->name();
        if (name.contains("键盘 - ")) {
            name = name.remove("键盘 - ");
        } else if(item->name().contains("Keyboard - ")) {
                name = name.remove("Keyboard - ");
        }
        item->setName(name);
        if (m_currentInputMethodList->isEmpty()) {
            m_currentInputMethodList->append(item);
        } else {
            m_currentInputMethodList->insert(1, item);
        }
        FcitxQtStringKeyValue imEntry;
        imEntry.setKey(item->uniqueName());
        m_imEntries.push_back(imEntry);

        row++;
    }
    emit imListChanged();
}

int IMConfig::addSelectedIM(const QModelIndex &index, QString matchStr) {
    int count = 0;
    qInfo("====> m_mode [%d]", m_mode);
    if (!index.isValid()) {
        return count;
    }

    if (!index.parent().isValid()) {
        if (m_mode == Tree) {
            int row_index = index.data(FcitxRowIndexRole).toInt();
            qInfo("row_index [%d]", row_index);

            m_currentIMEntries.clear();
            m_currentUseIMEntries.clear();
            FcitxQtStringKeyValueList useIMList = getUseIMList();
            if (matchStr == "") {
                ((AvailIMModel*)m_internalAvailIMModel)->getInputMethodEntryList(row_index, m_currentIMEntries, m_currentUseIMEntries, useIMList);
            } else {
                ((AvailIMModel*)m_internalAvailIMModel)->getInputMethodEntryList(row_index, m_currentIMEntries, m_currentUseIMEntries, useIMList, matchStr);
            }
            count = m_currentIMEntries.count();
        } else {
            qWarning("ERROR: m_mode != Tree. m_mode [%d]", m_mode);
        }

        updateIMList();
        emitChanged();
    }

    qInfo("<==== count [%d]", count);
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
        emit groupsChanged(m_groups);
    }

    if (!m_groups.empty()) {
        setCurrentGroup(m_groups.front());
    }
}

void IMConfig::reloadIMList()
{
    auto old = m_imEntries;
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
    m_imEntries = newEntries;
    setUseIMList(m_imEntries);
    updateIMList(true);
    emitChanged();
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
}

void IMConfig::fetchInputMethodsFinished(QDBusPendingCallWatcher *watcher) {
    qInfo("====>");
    QDBusPendingReply<FcitxQtInputMethodEntryList> ims = *watcher;
    watcher->deleteLater();
    if (!ims.isError()) {
        m_allIMs = ims.value();
        updateIMList();
    }

    qInfo("<====");
    return;
}

void IMConfig::setCurrentGroup(const QString &name) {
    qInfo("====> name [%s]", name.toStdString().c_str());
    if (m_dbus->available() && !name.isEmpty()) {
        auto call = m_dbus->controller()->InputMethodGroupInfo(name);
        m_lastGroup = name;
        emit currentGroupChanged(m_lastGroup);
        auto watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                &IMConfig::fetchGroupInfoFinished);
    }
    qInfo("<====");
}

void IMConfig::fetchGroupInfoFinished(QDBusPendingCallWatcher *watcher) {
    qInfo("====>");
    watcher->deleteLater();
    m_needSave = false;
    QDBusPendingReply<QString, FcitxQtStringKeyValueList> reply = *watcher;
    FcitxQtStringKeyValueList useIMEntries;
    if (!reply.isError()) {
        m_defaultLayout = reply.argumentAt<0>();
        m_imEntries = reply.argumentAt<1>();
        useIMEntries = reply.argumentAt<1>();
    } else {
        m_defaultLayout.clear();
        m_imEntries.clear();
        useIMEntries.clear();
    }
    emit defaultLayoutChanged();
    qInfo("----> defaultLayout_ [%s]", m_defaultLayout.toStdString().c_str());
    for (const auto& item : useIMEntries) {
        qInfo("----> key [%s], item [%s]",
              item.key().toStdString().c_str(),
              item.value().toStdString().c_str());
    }

    setUseIMList(useIMEntries);

    updateIMList();
    qInfo("<====");
}

void IMConfig::emitChanged() {
    m_needSave = true;
    emit changed();
}

void IMConfig::updateIMList(bool excludeCurrent) {
    qInfo("====> excludeCurrent [%d]", excludeCurrent);
    if (!excludeCurrent) {
        filterIMEntryList(m_allIMs, m_imEntries);
        m_currentIMModel->filterIMEntryList(m_allIMs, m_currentIMEntries);
    }
    m_internalAvailIMModel->filterIMEntryList(m_allIMs, m_imEntries);
    m_availIMModel->filterIMEntryList(m_allIMs, m_imEntries);

    emit imListChanged();
    qInfo("<====");
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

                QString name = item->name();
                if (name.contains("键盘 - ")) {
                    name = name.remove("键盘 - ");
                } else if(item->name().contains("Keyboard - ")) {
                        name = name.remove("Keyboard - ");
                }
                item->setName(name);

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
    if (m_dbus->controller()) {
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
}
