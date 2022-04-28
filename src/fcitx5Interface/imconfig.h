/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef CONFIGLIB_ABSTRACTIMPAGE_H
#define CONFIGLIB_ABSTRACTIMPAGE_H

#include <QDBusPendingCallWatcher>
#include <QObject>
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

    FcitxQtInputMethodItemList *currentIMModel() const { return m_currentInputMethodList; }
    IMProxyModel *availIMModel() const { return availIMModel_; }
    const QStringList &groups() const { return groups_; }
    const QString &currentGroup() const { return lastGroup_; }
    void setCurrentGroup(const QString &name);
    bool needSave() const { return needSave_; }

    //void addIM(FcitxQtInputMethodItem* item);
    //void removeIM(int index);

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

    FcitxQtInputMethodItemList* getFcitxQtInputMethodItemList() {return m_currentInputMethodList;}

public slots:
    void addGroup(const QString &name);
    void deleteGroup(const QString &name);
    void save();
    void load();
    void defaults();
    //void addIM(int index);
    void addIM(FcitxQtInputMethodItem* item);
    void removeIM(int index);
    void move(int from, int to);

signals:
    void changed();
    void currentGroupChanged(const QString &group);
    void groupsChanged(const QStringList &groups);
    void imListChanged();
    void defaultLayoutChanged();

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
    FcitxQtStringKeyValueList imEntries_;
    FcitxQtInputMethodEntryList allIMs_;
    QStringList groups_;
    QString lastGroup_;
    bool needSave_ = false;
};

#endif // CONFIGLIB_ABSTRACTIMPAGE_H
