/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "dbusprovider.h"
#include "logging.h"

namespace fcitx {
namespace kcm {

DBusProvider::DBusProvider(QObject *parent)
    : QObject(parent),
      watcher_(new FcitxQtWatcher(QDBusConnection::sessionBus(), this)) {
    qCDebug(KCM_FCITX5) << "Initializing DBusProvider with session bus";
    qCDebug(KCM_FCITX5) << "Creating FcitxQtWatcher instance";
    registerFcitxQtDBusTypes();
    qCDebug(KCM_FCITX5) << "Registered FcitxQt DBus types";
    connect(watcher_, &FcitxQtWatcher::availabilityChanged, this,
            &DBusProvider::fcitxAvailabilityChanged);
    watcher_->watch();
}

DBusProvider::~DBusProvider() {
    qCDebug(KCM_FCITX5) << "Destroying DBusProvider";
    watcher_->unwatch();
    qCDebug(KCM_FCITX5) << "Stopped watching Fcitx DBus service";
}

void DBusProvider::fcitxAvailabilityChanged(bool avail) {
    qCInfo(KCM_FCITX5) << "Fcitx DBus availability changed to:" << avail;
    delete controller_;
    controller_ = nullptr;

    if (avail) {
        qCInfo(KCM_FCITX5) << "Fcitx DBus service available, service name:"
                         << watcher_->serviceName();
        qCDebug(KCM_FCITX5) << "Creating new FcitxQtControllerProxy with timeout: 3000ms";
        controller_ =
            new FcitxQtControllerProxy(watcher_->serviceName(), "/controller",
                                       watcher_->connection(), this);
        controller_->setTimeout(3000);
    } else {
        qCDebug(KCM_FCITX5) << "Fcitx DBus became unavailable, cleared controller";
    }

    Q_EMIT availabilityChanged(controller_);
}

} // namespace kcm
} // namespace fcitx
