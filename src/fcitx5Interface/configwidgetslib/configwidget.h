// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _KCM_FCITX_CONFIGWIDGET_H_
#define _KCM_FCITX_CONFIGWIDGET_H_

#include <QDialog>
#include <QDialogButtonBox>
#include <QWidget>
#include <fcitxqtdbustypes.h>

class QDBusPendingCallWatcher;
class QFormLayout;
class DBusProvider;

class ConfigWidget : public QWidget {
    Q_OBJECT

public:
    explicit ConfigWidget(const QString &uri, DBusProvider *module,
                          QWidget *parent = 0);

    explicit ConfigWidget(const QMap<QString, fcitx::FcitxQtConfigOptionList> &desc,
                          QString mainType, DBusProvider *module,
                          QWidget *parent = 0);

    static QDialog *configDialog(QWidget *parent, DBusProvider *module,
                                 const QString &uri,
                                 const QString &title = QString());

    auto dbus() { return dbus_; }
    auto &description() const { return desc_; }

Q_SIGNALS:
    void changed();

public Q_SLOTS:
    void load();
    void save();
    void buttonClicked(QDialogButtonBox::StandardButton);

    QVariant value() const;
    void setValue(const QVariant &variant);
    void requestConfig(bool sync = false);
private Q_SLOTS:
    void requestConfigFinished(QDBusPendingCallWatcher *watcher);
    void doChanged();

private:
    void setupWidget(QWidget *widget, const QString &type, const QString &path);
    void addOptionWidget(QFormLayout *layout, const fcitx::FcitxQtConfigOption &option,
                         const QString &path);

    bool initialized_ = false;
    QString uri_;
    QMap<QString, fcitx::FcitxQtConfigOptionList> desc_;
    QString mainType_;
    DBusProvider *dbus_;
    QWidget *mainWidget_;

    bool dontEmitChanged_ = false;
};

//extern ConfigWidget *getConfigWidget(QWidget *widget);

#endif // _KCM_FCITX_CONFIGWIDGET_H_
