/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef IMCONFIG_H
#define IMCONFIG_H

#include <QDBusPendingCallWatcher>
#include <QObject>
#include "dbusprovider.h"
#include <fcitxqtdbustypes.h>

#include "publisher/publisherdef.h"

class IMConfigModelInterface;
class IMProxyModel;
class FilteredIMModel;
class DBusProvider;

using namespace fcitx;

class IMConfig : public QObject {
    Q_OBJECT
public:
    enum ModelMode { Tree, Flatten };

    IMConfig(DBusProvider *dbus, ModelMode mode, QObject *parent);
    ~IMConfig();

    void testAddIMDeal(int imIndex);

    FilteredIMModel *currentFilteredIMModel() const { return m_currentIMModel; }
    FcitxQtInputMethodItemList *currentIMModel() const { return m_currentInputMethodList; }
    IMProxyModel *availIMModel() const { return m_availIMModel; }
    const QStringList &groups() const { return m_groups; }
    const QString &currentGroup() const { return m_lastGroup; }
    void setCurrentGroup(const QString &name);
    bool needSave() const { return m_needSave; }

    //void addIM(FcitxQtInputMethodItem* item);
    //void removeIM(int index);

    int addSelectedIM(int index, QString matchStr = "");
    int addSelectedIM(const QModelIndex& index, QString matchStr = "");

    const auto& currentIMEntries() const { return m_currentIMEntries; }
    const auto& currentUseIMEntries() const { return m_currentUseIMEntries; }
    const auto &imEntries() const { return m_imEntries; }
    const auto &allIms() const { return m_allIMs; }
    void setIMEntries(const FcitxQtStringKeyValueList &imEntires) {
        m_imEntries = imEntires;
        updateIMList();
    }

    void clearCurrentIMEntries()
    {
        m_currentIMEntries.clear();
        updateIMList();
    }

    const QString &defaultLayout() const { return m_defaultLayout; }
    void setDefaultLayout(const QString &l) {
        if (m_defaultLayout != l) {
            m_defaultLayout = l;
            emitChanged();
            emit defaultLayoutChanged();
        }
    }

    Q_INVOKABLE void setLayout(const QString &im, const QString &layout) {
        for (auto &imEntry : m_imEntries) {
            if (imEntry.key() == im) {
                imEntry.setValue(layout);
                emitChanged();
                updateIMList();
                return;
            }
        }
    }

    void emitChanged();

    void load_test_data();

    FcitxQtInputMethodItemList* getFcitxQtInputMethodItemList() {return m_currentInputMethodList;}

public slots:
    void addGroup(const QString &name);
    void deleteGroup(const QString &name);
    void save();
    void saveSelectedIM(int imIndex);
    void load();
    void defaults();
    //void addIM(int index);
    void addIM(const QString& name);
    void removeIM(int index);
    void move(int from, int to);

signals:
    void changed();
    void currentGroupChanged(const QString &group);
    void groupsChanged(const QStringList &groups);
    void imListChanged();
    void defaultLayoutChanged();
    void addIMSignal(int imIndex);

private slots:
    void availabilityChanged();
    void fetchGroupInfoFinished(QDBusPendingCallWatcher *watcher);
    void fetchInputMethodsFinished(QDBusPendingCallWatcher *watcher);
    void fetchGroupsFinished(QDBusPendingCallWatcher *watcher);

private:
    void reloadIMList();
    void reloadGroup();
    void updateIMList(bool excludeCurrent = false);
    void filterIMEntryList(
        const FcitxQtInputMethodEntryList &imEntryList,
        const FcitxQtStringKeyValueList &enabledIMList);

    DBusProvider *m_dbus;
    IMProxyModel *m_availIMModel;
    IMConfigModelInterface *m_internalAvailIMModel = nullptr;
    FcitxQtInputMethodItemList *m_currentInputMethodList;
    QString m_defaultLayout;

    FilteredIMModel *m_currentIMModel;
    FcitxQtStringKeyValueList m_currentIMEntries;
    FcitxQtStringKeyValueList m_currentUseIMEntries;

    FcitxQtStringKeyValueList m_imEntries;
    FcitxQtInputMethodEntryList m_allIMs;
    QStringList m_groups;
    QString m_lastGroup;
    bool m_needSave = false;

    int m_mode;
};

#endif
