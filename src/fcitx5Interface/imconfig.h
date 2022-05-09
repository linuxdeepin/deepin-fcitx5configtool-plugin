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
    IMProxyModel *availIMModel() const { return availIMModel_; }
    const QStringList &groups() const { return groups_; }
    const QString &currentGroup() const { return lastGroup_; }
    void setCurrentGroup(const QString &name);
    bool needSave() const { return needSave_; }

    //void addIM(FcitxQtInputMethodItem* item);
    //void removeIM(int index);

    void addSelectedIM(const QModelIndex& index);

    const auto& currentIMEntries() const { return m_currentIMEntries; }
    const auto &imEntries() const { return imEntries_; }
    void setIMEntries(const FcitxQtStringKeyValueList &imEntires) {
        imEntries_ = imEntires;
        updateIMList();
    }

    const QString &defaultLayout() const { return defaultLayout_; }
    void setDefaultLayout(const QString &l) {
        if (defaultLayout_ != l) {
            defaultLayout_ = l;
            emitChanged();
            emit defaultLayoutChanged();
        }
    }

    Q_INVOKABLE void setLayout(const QString &im, const QString &layout) {
        for (auto &imEntry : imEntries_) {
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
    void addIM(FcitxQtInputMethodItem* item);
    void addSelectedIM(int index);
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

    DBusProvider *dbus_;
    IMProxyModel *availIMModel_;
    IMConfigModelInterface *internalAvailIMModel_ = nullptr;
    FcitxQtInputMethodItemList *m_currentInputMethodList;
    QString defaultLayout_;

    FilteredIMModel *m_currentIMModel;
    FcitxQtStringKeyValueList m_currentIMEntries;

    FcitxQtStringKeyValueList imEntries_;
    FcitxQtInputMethodEntryList allIMs_;
    QStringList groups_;
    QString lastGroup_;
    bool needSave_ = false;

    int m_mode;
};

#endif
