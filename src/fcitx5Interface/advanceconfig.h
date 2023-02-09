// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FCITX_ADVANCECONFIG_H_
#define FCITX_ADVANCECONFIG_H_

#include <QDialog>
#include <QDialogButtonBox>
#include <QWidget>
#include <fcitxqtdbustypes.h>
#include <fcitx-utils/key.h>

class QDBusPendingCallWatcher;
class QFormLayout;

class DBusProvider;

class AdvanceConfig : public QObject {
    Q_OBJECT

public:
    explicit AdvanceConfig(const QString &uri, DBusProvider *module,
                          QObject *parent = nullptr);

    static QDialog *configDialog(QWidget *parent, DBusProvider *module,
                                 const QString &uri,
                                 const QString &title = QString());

    auto dbus() { return m_dbus; }
    auto &description() const { return m_desc; }
    void switchIMShortCuts(const QString& shortCuts);
    void switchFirstIMShortCuts(const QString& shortCuts);
    void disableSwitchIMShortCutsFunc(const bool isDisable);
signals:
    void changed();
    void switchIMShortCutsChanged(const QString& shortCuts);
    void switchFirstIMShortCutsChanged(const QString& shortCuts);

public slots:
    void load();
    void save();
    void buttonClicked(QDialogButtonBox::StandardButton);

    QVariant value() const;
    void setValue(const QVariant &variant);

    void requestConfig(bool sync = false);
private slots:
    void requestConfigFinished(QDBusPendingCallWatcher *watcher);
    void doChanged();

private:
    void setupData(const QString &type, const QString &path);
    void addChildData(const fcitx::FcitxQtConfigOption &option, const QString &path);
    QList<fcitx::Key> readValue(const QVariantMap &map, const QString &path);

    bool m_initialized = false;
    QString m_uri;
    QMap<QString, fcitx::FcitxQtConfigOptionList> m_desc;
    QString m_mainType;
    DBusProvider *m_dbus;
    bool m_dontEmitChanged {false};
    QString m_switchIMShortCuts;
    QString m_switchFirstIMShortCuts;
    QWidget *m_configWidget;
    bool    isDisableSwitchIMShortCutsFunc;
};

AdvanceConfig *getConfigWidget(QWidget *widget);

#endif // FCITX_ADVANCECONFIG_H_
