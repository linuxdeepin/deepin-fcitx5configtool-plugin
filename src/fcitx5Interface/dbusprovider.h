/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef DBUSPROVIDER_H
#define DBUSPROVIDER_H

#include <QObject>
#include <fcitxqtcontrollerproxy.h>
#include <fcitxqtwatcher.h>

using namespace fcitx;

class DBusProvider : public QObject {
    Q_OBJECT

public:
    DBusProvider(QObject *parent);
    ~DBusProvider();

    bool available() const { return m_controller != nullptr; }
    FcitxQtControllerProxy *controller() { return m_controller; }

signals:
    void availabilityChanged(bool avail);

private slots:
    void fcitxAvailabilityChanged(bool avail);

private:
    FcitxQtWatcher *m_watcher;
    FcitxQtControllerProxy *m_controller = nullptr;
};

#endif
