// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "fcitx5configtool.h"
#include "private/fcitx5configtool_p.h"
#include "fcitx5configproxy.h"

#include "dbusprovider.h"
#include "imconfig.h"
#include "model.h"

#include "dde-control-center/dccfactory.h"

#include <fcitxqtdbustypes.h>

#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QtQml/QQmlEngine>

namespace deepin {
namespace fcitx5configtool {

static QString kFcitxConfigGlobalPath = "fcitx://config/global";

Fcitx5ConfigToolWorkerPrivate::Fcitx5ConfigToolWorkerPrivate(Fcitx5ConfigToolWorker *parent)
    : QObject(parent), q(parent)
{
    
}

void Fcitx5ConfigToolWorkerPrivate::init()
{
    dbusProvider = new fcitx::kcm::DBusProvider(this);
    imConfig = new fcitx::kcm::IMConfig(dbusProvider, fcitx::kcm::IMConfig::Tree, this);
    configProxy = new Fcitx5ConfigProxy(dbusProvider, kFcitxConfigGlobalPath, this);
    initConnect();
}

void Fcitx5ConfigToolWorkerPrivate::initConnect()
{
    connect(imConfig, &fcitx::kcm::IMConfig::imListChanged, this, [=]() {
        qInfo() << "list changed:" << imConfig->currentIMModel()->rowCount();
    });
    connect(configProxy, &Fcitx5ConfigProxy::requestConfigFinished, q, &Fcitx5ConfigToolWorker::requestConfigFinished);
    configProxy->requestConfig(false);
}

Fcitx5ConfigToolWorker::Fcitx5ConfigToolWorker(QObject *parent)
    : QObject(parent), d(new Fcitx5ConfigToolWorkerPrivate(this))
{
    qmlRegisterType<Fcitx5ConfigProxy>("com.deepin.dcc.fcitx5configtool", 1, 0, "Fcitx5ConfigProxy");
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

void Fcitx5ConfigToolWorker::init()
{
    d->init();
}

Fcitx5ConfigProxy *Fcitx5ConfigToolWorker::fcitx5ConfigProxy() const
{
    return d->configProxy;
}

DCC_FACTORY_CLASS(Fcitx5ConfigToolWorker)
} // namespace fcitx5configtool
} // namespace deepin

#include "fcitx5configtool.moc"
