// SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

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
    Q_PROPERTY(FilteredIMModel *currentIMModel READ currentIMModel
                   CONSTANT)
    Q_PROPERTY(IMProxyModel *availIMModel READ availIMModel CONSTANT)
    Q_PROPERTY(QString defaultLayout READ defaultLayout WRITE setDefaultLayout
                   NOTIFY defaultLayoutChanged)
    Q_PROPERTY(QStringList groups READ groups NOTIFY groupsChanged)
    Q_PROPERTY(QString currentGroup READ currentGroup WRITE setCurrentGroup)
    Q_PROPERTY(bool needSave READ needSave)
    Q_PROPERTY(bool needUpdate READ needUpdate NOTIFY needUpdateChanged)
public:
    enum ModelMode { Tree, Flatten };

    IMConfig(DBusProvider *dbus, ModelMode mode, QObject *parent);
    ~IMConfig();

    FilteredIMModel *currentIMModel() const { return m_currentIMModel; }
    IMProxyModel *availIMModel() const { return m_availIMModel; }
    const QStringList &groups() const { return m_groups; }
    const QString &currentGroup() const { return m_lastGroup; }
    void setCurrentGroup(const QString &name);
    bool needSave() const { return m_needSave; }
    bool needUpdate() const { return m_needUpdate; }

    void addIM(const QModelIndex &index);
    void addIMs(const QModelIndexList &indexes);
    void removeIM(const QModelIndex &index);

    const auto &imEntries() const { return m_imEntries; }
    void setIMEntries(const FcitxQtStringKeyValueList &imEntires) {
        m_imEntries = imEntires;
        updateIMList();
    }

    const QString &defaultLayout() const { return m_defaultLayout; }
    void setDefaultLayout(const QString &l) {
        if (m_defaultLayout != l) {
            m_defaultLayout = l;
            emitChanged();
            Q_EMIT defaultLayoutChanged();
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

public Q_SLOTS:
    void addGroup(const QString &name);
    void deleteGroup(const QString &name);
    void save();
    void load();
    void defaults();
    void addIM(int index);
    void removeIM(int index);
    void move(int from, int to);
    void refresh();
    void restart();

Q_SIGNALS:
    void changed();
    void currentGroupChanged(const QString &group);
    void groupsChanged(const QStringList &groups);
    void imListChanged();
    void defaultLayoutChanged();
    void needUpdateChanged(bool);

private Q_SLOTS:
    void availabilityChanged();
    void fetchGroupInfoFinished(QDBusPendingCallWatcher *watcher);
    void fetchInputMethodsFinished(QDBusPendingCallWatcher *watcher);
    void fetchGroupsFinished(QDBusPendingCallWatcher *watcher);
    void checkUpdateFinished(QDBusPendingCallWatcher *watcher);

private:
    void reloadGroup();
    void updateIMList(bool excludeCurrent = false);

    DBusProvider *m_dbus;
    IMProxyModel *m_availIMModel;
    IMConfigModelInterface *m_internalAvailIMModel = nullptr;
    FilteredIMModel *m_currentIMModel;
    QString m_defaultLayout;
    FcitxQtStringKeyValueList m_imEntries;
    FcitxQtInputMethodEntryList m_allIMs;
    QStringList m_groups;
    QString m_lastGroup;
    bool m_needSave = false;
    bool m_needUpdate = false;
};

#endif
