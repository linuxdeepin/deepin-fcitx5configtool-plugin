// SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusprovider.h"

DBusProvider::DBusProvider(QObject *parent)
    : QObject(parent)
    , m_watcher(new FcitxQtWatcher(QDBusConnection::sessionBus(), this))
{
    registerFcitxQtDBusTypes();
    connect(m_watcher, &FcitxQtWatcher::availabilityChanged, this,
            &DBusProvider::fcitxAvailabilityChanged);
    m_watcher->watch();
}

DBusProvider::~DBusProvider() { m_watcher->unwatch(); }

void DBusProvider::fcitxAvailabilityChanged(bool avail) {
    delete m_controller;
    m_controller = nullptr;

    if (avail) {
        m_controller =
            new FcitxQtControllerProxy(m_watcher->serviceName(), "/controller",
                                       m_watcher->connection(), this);
        m_controller->setTimeout(3000);
    }

    emit availabilityChanged(m_controller);
}
