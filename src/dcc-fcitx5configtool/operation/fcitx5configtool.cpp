// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "fcitx5configtool.h"
#include "private/fcitx5configtool_p.h"
#include "fcitx5configproxy.h"

#include <dbusprovider.h>
#include <imconfig.h>
#include <model.h>
#include <configwidget.h>

#include <dde-control-center/dccfactory.h>

#include <fcitxqtdbustypes.h>

#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QtQml/QQmlEngine>

#include <DDBusSender>
#include <DUtil>

namespace deepin {
namespace fcitx5configtool {

static QString kFcitxConfigGlobalPath = "fcitx://config/global";
static const QString kSogouIMUniqueName = "com.sogou.ime.ng.fcitx5.deepin";
static const QString kSogouConfigureAppId = "com.sogou.ime.ng.fcitx5.deepin.configurer";

Fcitx5ConfigToolWorkerPrivate::Fcitx5ConfigToolWorkerPrivate(Fcitx5ConfigToolWorker *parent)
    : QObject(parent), q(parent)
{
}

void Fcitx5ConfigToolWorkerPrivate::init()
{
    dbusProvider = new fcitx::kcm::DBusProvider(this);
    imConfig = new fcitx::kcm::IMConfig(dbusProvider, fcitx::kcm::IMConfig::Tree, this);
    configProxy = new Fcitx5ConfigProxy(dbusProvider, kFcitxConfigGlobalPath, this);
    imListModel = new IMListModel(this);
    imListModel->resetData(imConfig->currentIMModel());

    initConnect();
}

void Fcitx5ConfigToolWorkerPrivate::initConnect()
{
    connect(dbusProvider, &fcitx::kcm::DBusProvider::availabilityChanged, this, [this](bool avail) {
        qInfo() << "Availability changed:" << avail;
        if (avail) {
            configProxy->requestConfig(false);
        } else {
            configProxy->clear();
        }
    });
    connect(imConfig, &fcitx::kcm::IMConfig::imListChanged, this, [=]() {
        qInfo() << "list changed:" << imConfig->currentIMModel()->rowCount();
        imListModel->resetData(imConfig->currentIMModel());
    });

    connect(imListModel, &IMListModel::requestRemove, this, [this](int index) {
        qInfo() << "Remove IM:" << index;
        imConfig->removeIM(index);
        imConfig->save();
    });

    connect(imListModel, &IMListModel::requestMove, this, [this](int from, int to) {
        qInfo() << "Move IM from" << from << to;
        imConfig->move(from, to);
        imConfig->save();
    });

    connect(configProxy, &Fcitx5ConfigProxy::requestConfigFinished, q, &Fcitx5ConfigToolWorker::requestConfigFinished);
    configProxy->requestConfig(false);
}

Fcitx5ConfigToolWorker::Fcitx5ConfigToolWorker(QObject *parent)
    : QObject(parent), d(new Fcitx5ConfigToolWorkerPrivate(this))
{
    qmlRegisterType<Fcitx5ConfigProxy>("org.deepin.dcc.fcitx5configtool", 1, 0, "Fcitx5ConfigProxy");
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

void Fcitx5ConfigToolWorker::openDeepinAppStore() const
{
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

void Fcitx5ConfigToolWorker::showIMSettingsDialog(int index)
{
    if (!d->imConfig)
        return;

    auto item = d->imConfig->currentIMModel()->index(index);
    QString uniqueName = item.data(fcitx::kcm::FcitxIMUniqueNameRole).toString();

    if (uniqueName == kSogouIMUniqueName) {
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

    // TODO(zhangs): create dialog by a exec app
    // QString title = item.data(Qt::DisplayRole).toString();
    // QPointer<QDialog> dialog = fcitx::kcm::ConfigWidget::configDialog(
    //         nullptr,
    //         d->dbusProvider,
    //         QString("fcitx://config/inputmethod/%1").arg(uniqueName),
    //         title);
    // dialog->exec();
    // delete dialog;
}

IMListModel *Fcitx5ConfigToolWorker::imlistModel() const
{
    Q_ASSERT(d->imListModel);
    return d->imListModel;
}

DCC_FACTORY_CLASS(Fcitx5ConfigToolWorker)
}   // namespace fcitx5configtool
}   // namespace deepin

#include "fcitx5configtool.moc"
