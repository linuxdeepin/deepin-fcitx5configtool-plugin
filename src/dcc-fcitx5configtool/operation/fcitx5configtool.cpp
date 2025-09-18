// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "fcitx5configtool.h"
#include "private/fcitx5configtool_p.h"
#include "fcitx5configproxy.h"
#include "fcitx5addonsproxy.h"

#include <dbusprovider.h>
#include <imconfig.h>
#include <configwidget.h>

#include <dde-control-center/dccfactory.h>

#include <fcitxqtdbustypes.h>

#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QtQml/QQmlEngine>
#include <QLoggingCategory>

#include <DDBusSender>
#include <DUtil>
#include <QFileInfo>

Q_LOGGING_CATEGORY(configTool, "fcitx5.configtool.main")

namespace deepin {
namespace fcitx5configtool {

static QString kFcitxConfigGlobalPath = "fcitx://config/global";
static const QString kSogouAddonUniqueName = "com.sogou.ime.ng.fcitx5.deepin-addon";
static const QString kSogouIMUniqueName = "com.sogou.ime.ng.fcitx5.deepin";
static const QString kSogouConfigureAppId = "com.sogou.ime.ng.fcitx5.deepin.configurer";

Fcitx5ConfigToolWorkerPrivate::Fcitx5ConfigToolWorkerPrivate(Fcitx5ConfigToolWorker *parent)
    : QObject(parent), q(parent)
{
    qCDebug(configTool) << "Entering Fcitx5ConfigToolWorkerPrivate constructor";
}

void Fcitx5ConfigToolWorkerPrivate::init()
{
    qCDebug(configTool) << "Entering Fcitx5ConfigToolWorkerPrivate::init";
    dbusProvider = new fcitx::kcm::DBusProvider(this);
    imConfig = new fcitx::kcm::IMConfig(dbusProvider, fcitx::kcm::IMConfig::Flatten, this);
    configProxy = new Fcitx5ConfigProxy(dbusProvider, kFcitxConfigGlobalPath, this);
    addonsProxy = new Fcitx5AddonsProxy(dbusProvider, this);
    imListModel = new IMListModel(this);
    imListModel->resetData(imConfig->currentIMModel());

    initConnect();
    qCDebug(configTool) << "Exiting Fcitx5ConfigToolWorkerPrivate::init";
}

void Fcitx5ConfigToolWorkerPrivate::initConnect()
{
    qCDebug(configTool) << "Entering Fcitx5ConfigToolWorkerPrivate::initConnect";
    connect(dbusProvider, &fcitx::kcm::DBusProvider::availabilityChanged, this, [this](bool avail) {
        qCInfo(configTool) << "DBus availability changed:" << avail;
        if (avail) {
            configProxy->requestConfig(false);
            addonsProxy->load();
        } else {
            configProxy->clear();
            addonsProxy->clear();
        }
    });
    connect(imConfig, &fcitx::kcm::IMConfig::imListChanged, this, [=]() {
        qCInfo(configTool) << "IM list changed, resetting model. New count:" << imConfig->currentIMModel()->rowCount();
        imListModel->resetData(imConfig->currentIMModel());
    });

    connect(imListModel, &IMListModel::requestRemove, this, [this](int index) {
        qCInfo(configTool) << "Request to remove IM at index:" << index;
        imConfig->removeIM(index);
        imConfig->save();
    });

    connect(imListModel, &IMListModel::requestMove, this, [this](int from, int to) {
        qCInfo(configTool) << "Request to move IM from" << from << "to" << to;
        imConfig->move(from, to);
        imConfig->save();
    });

    connect(configProxy, &Fcitx5ConfigProxy::requestConfigFinished, q, &Fcitx5ConfigToolWorker::requestConfigFinished);
    connect(addonsProxy, &Fcitx5AddonsProxy::requestAddonsFinished, q, &Fcitx5ConfigToolWorker::requestAddonsFinished);
    configProxy->requestConfig(false);
    addonsProxy->load();
    qCDebug(configTool) << "Exiting Fcitx5ConfigToolWorkerPrivate::initConnect";
}

Fcitx5ConfigToolWorker::Fcitx5ConfigToolWorker(QObject *parent)
    : QObject(parent), d(new Fcitx5ConfigToolWorkerPrivate(this))
{
    qCDebug(configTool) << "Entering Fcitx5ConfigToolWorker constructor";
    qmlRegisterType<Fcitx5ConfigProxy>("org.deepin.dcc.fcitx5configtool", 1, 0, "Fcitx5ConfigProxy");
    qmlRegisterType<Fcitx5AddonsProxy>("org.deepin.dcc.fcitx5configtool", 1, 0, "Fcitx5AddonsProxy");
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
    qCDebug(configTool) << "Exiting Fcitx5ConfigToolWorker constructor";
}

void Fcitx5ConfigToolWorker::init()
{
    qCDebug(configTool) << "Entering Fcitx5ConfigToolWorker::init";
    d->init();
    qCDebug(configTool) << "Exiting Fcitx5ConfigToolWorker::init";
}

Fcitx5ConfigProxy *Fcitx5ConfigToolWorker::fcitx5ConfigProxy() const
{
    // qCDebug(configTool) << "Accessing fcitx5ConfigProxy";
    return d->configProxy;
}

Fcitx5AddonsProxy *Fcitx5ConfigToolWorker::fcitx5AddonsProxy() const
{
    // qCDebug(configTool) << "Accessing fcitx5AddonsProxy";
    return d->addonsProxy;
}

void Fcitx5ConfigToolWorker::showAddonSettingsDialog(const QString &addonStr, const QString &title) const
{
    qCDebug(configTool) << "Showing addon settings dialog for:" << addonStr << "title:" << title;
    // If addonStr is empty or addonsProxy is null, don't show the dialog
    if (addonStr.isEmpty() || !d->addonsProxy) {
        qCWarning(configTool) << "Invalid request: addon string is empty or addonsProxy is null.";
        return;
    }

    if (addonStr == kSogouAddonUniqueName) {
        qCInfo(configTool) << "Launching Sogou specific configurer.";
        DDBusSender()
                .service("org.desktopspec.ApplicationManager1")
                .path(QStringLiteral("/org/desktopspec/ApplicationManager1/") + DUtil::escapeToObjectPath(kSogouConfigureAppId))
                .interface("org.desktopspec.ApplicationManager1.Application")
                .method("Launch")
                .arg(QString(""))
                .arg(QStringList())
                .arg(QVariantMap())
                .call();
        return;
    }

    qCInfo(configTool) << "Launching generic addon config dialog.";
    launchConfigDialog(QString("fcitx://config/addon/%1").arg(addonStr),
                       title);
}

void Fcitx5ConfigToolWorker::openDeepinAppStore() const
{
    qCInfo(configTool) << "Request to open Deepin App Store.";
    DDBusSender()
            .service("org.desktopspec.ApplicationManager1")
            .path(QStringLiteral("/org/desktopspec/ApplicationManager1/") + DUtil::escapeToObjectPath("deepin-app-store"))
            .interface("org.desktopspec.ApplicationManager1.Application")
            .method("Launch")
            .arg(QString(""))
            .arg(QStringList())
            .arg(QVariantMap())
            .call();
}

void Fcitx5ConfigToolWorker::showIMSettingsDialog(int index) const
{
    qCInfo(configTool) << "Request to show IM settings dialog for index:" << index;
    if (!d->imConfig) {
        qCWarning(configTool) << "Cannot show IM settings dialog: imConfig is null.";
        return;
    }

    auto item = d->imConfig->currentIMModel()->index(index);
    QString uniqueName = item.data(fcitx::kcm::FcitxIMUniqueNameRole).toString();

    if (uniqueName == kSogouIMUniqueName) {
        qCInfo(configTool) << "Launching Sogou specific IM configurer.";
        DDBusSender()
                .service("org.desktopspec.ApplicationManager1")
                .path(QStringLiteral("/org/desktopspec/ApplicationManager1/") + DUtil::escapeToObjectPath(kSogouConfigureAppId))
                .interface("org.desktopspec.ApplicationManager1.Application")
                .method("Launch")
                .arg(QString(""))
                .arg(QStringList())
                .arg(QVariantMap())
                .call();
        return;
    }

    qCInfo(configTool) << "Launching generic IM config dialog for:" << uniqueName;
    launchConfigDialog(QString("fcitx://config/inputmethod/%1").arg(uniqueName),
                       item.data(Qt::DisplayRole).toString());
}

void Fcitx5ConfigToolWorker::addIM(int index)
{
    qCInfo(configTool) << "Request to add IM at index:" << index;
    d->imConfig->addIM(index);
    d->imConfig->save();
    qCInfo(configTool) << "IM added and saved successfully.";
}

void Fcitx5ConfigToolWorker::launchConfigDialog(const QString &uri, const QString &title) const
{
    qCDebug(configTool) << "Entering launchConfigDialog for uri:" << uri << "with title:" << title;
    QString execPath = QString::fromLocal8Bit(DCC_CONFIGTOOL_EXEC_PATH) + "/dcc-fcitx5configtool-exec";
    QFileInfo fileInfo(execPath);
    if (!fileInfo.exists()) {
        qCWarning(configTool) << "Primary exec path not found:" << execPath << "Using fallback.";
        execPath = "/usr/libexec/dcc-fcitx5configtool-exec";
    }

    QStringList args;
    args << "-u" << uri
         << "-t" << title;
    qCInfo(configTool) << "Executing config dialog:" << execPath << "with args:" << args;
    QProcess::startDetached(execPath, args);
    qCDebug(configTool) << "Exiting launchConfigDialog";
}

IMListModel *Fcitx5ConfigToolWorker::imlistModel() const
{
    qCDebug(configTool) << "Accessing imlistModel";
    Q_ASSERT(d->imListModel);
    return d->imListModel;
}

fcitx::kcm::IMProxyModel *Fcitx5ConfigToolWorker::imProxyModel() const
{
    qCDebug(configTool) << "Accessing imProxyModel";
    return d->imConfig->availIMModel();
}

DCC_FACTORY_CLASS(Fcitx5ConfigToolWorker)
}   // namespace fcitx5configtool
}   // namespace deepin

#include "fcitx5configtool.moc"
