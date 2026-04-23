// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "fcitx5configtool.h"
#include "private/fcitx5configtool_p.h"
#include "fcitx5configproxy.h"
#include "fcitx5addonsproxy.h"
#include "keyboardcontroller.h"

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

static QString dccLayoutToFcitxIMKey(const QString &key) {
    // DCC: "fr;" or "fr;bepo" -> fcitx5: "keyboard-fr" or "keyboard-fr-bepo"
    QString stripped = key;
    stripped.replace(';', '-');
    while (stripped.endsWith('-')) stripped.chop(1);
    return QStringLiteral("keyboard-") + stripped;
}
static QString fcitxIMKeyToDccLayout(const QString &imKey) {
    const QString prefix = QStringLiteral("keyboard-");
    if (!imKey.startsWith(prefix)) return QString();
    // fcitx5: "keyboard-fr-bepo" -> DCC: "fr;bepo"
    QString stripped = imKey.mid(prefix.length());
    int dashPos = stripped.indexOf('-');
    if (dashPos < 0) {
        return stripped + ';';
    }
    return stripped.left(dashPos) + ';' + stripped.mid(dashPos + 1);
}

static int indexOfEntry(const fcitx::FcitxQtStringKeyValueList &entries, const QString &key) {
    for (int i = 0; i < entries.size(); ++i)
        if (entries[i].key() == key) return i;
    return -1;
}

struct SyncGuard {
    bool &flag;
    SyncGuard(bool &f) : flag(f) { flag = true; }
    ~SyncGuard() { flag = false; }
    SyncGuard(const SyncGuard &) = delete;
    SyncGuard &operator=(const SyncGuard &) = delete;
};

static QString kFcitxConfigGlobalPath = "fcitx://config/global";
#ifdef BUILD_UOS
static const QString kSogouAddonUniqueName = "com.sogou.ime.ng.fcitx5.uos-addon";
static const QString kSogouIMUniqueName = "com.sogou.ime.ng.fcitx5.uos";
static const QString kSogouConfigureAppId = "com.sogou.ime.ng.fcitx5.uos.configurer";
#else
static const QString kSogouAddonUniqueName = "com.sogou.ime.ng.fcitx5.deepin-addon";
static const QString kSogouIMUniqueName = "com.sogou.ime.ng.fcitx5.deepin";
static const QString kSogouConfigureAppId = "com.sogou.ime.ng.fcitx5.deepin.configurer";
#endif

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
    keyboardController = new dccV25::KeyboardController(this);

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
            connect(dbusProvider->controller(), &fcitx::FcitxQtControllerProxy::InputMethodGroupsChanged,
                    this, [this]() {
                if (!m_syncInProgress) imConfig->load();
            }, Qt::UniqueConnection);
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

    connect(keyboardController, &dccV25::KeyboardController::currentLayoutSet, this,
            [this](const QString &key) {
        if (!m_syncInProgress) syncCurrentLayoutToFcitx5(key);
    });

    connect(keyboardController, &dccV25::KeyboardController::layoutDeleted, this,
            [this](const QString &key, bool isCurrent) {
        if (!m_syncInProgress && isCurrent) syncLayoutDeletedFromFcitx5(key);
    });

    connect(imConfig, &fcitx::kcm::IMConfig::imListChanged, this, [this]() {
        if (!m_syncInProgress) syncFcitx5FirstKeyboardToDCC();
        if (!m_pendingLayoutKey.isEmpty()) {
            QString key = m_pendingLayoutKey;
            syncCurrentLayoutToFcitx5(key);
        }
    });

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

dccV25::KeyboardController *Fcitx5ConfigToolWorker::keyboardController() const
{
    return d->keyboardController;
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

void Fcitx5ConfigToolWorkerPrivate::syncCurrentLayoutToFcitx5(const QString &layoutKey)
{
    if (!dbusProvider || !dbusProvider->controller()) {
        qCDebug(configTool) << "syncCurrentLayoutToFcitx5: controller not available, pending:" << layoutKey;
        m_pendingLayoutKey = layoutKey;
        return;
    }

    QString fcitxKey = dccLayoutToFcitxIMKey(layoutKey);

    const auto &entries = imConfig->imEntries();
    int existIndex = indexOfEntry(entries, fcitxKey);

    SyncGuard guard(m_syncInProgress);
    m_pendingLayoutKey.clear();

    if (existIndex > 0) {
        qCDebug(configTool) << "syncCurrentLayoutToFcitx5: move to front:" << fcitxKey;
        auto updatedEntries = entries;
        updatedEntries.move(existIndex, 0);
        imConfig->setIMEntries(updatedEntries);
        imConfig->emitChanged();
        imConfig->save();
    } else if (existIndex < 0) {
        qCDebug(configTool) << "syncCurrentLayoutToFcitx5: prepend:" << fcitxKey;
        auto updatedEntries = entries;
        fcitx::FcitxQtStringKeyValue newEntry;
        newEntry.setKey(fcitxKey);
        updatedEntries.prepend(newEntry);
        imConfig->setIMEntries(updatedEntries);
        imConfig->emitChanged();
        imConfig->save();
    }

    dbusProvider->controller()->SetCurrentIM(fcitxKey);
}

void Fcitx5ConfigToolWorkerPrivate::syncLayoutDeletedFromFcitx5(const QString &layoutKey)
{
    if (!dbusProvider || !dbusProvider->controller()) return;

    QString fcitxKey = dccLayoutToFcitxIMKey(layoutKey);
    const auto &entries = imConfig->imEntries();
    int removedIndex = indexOfEntry(entries, fcitxKey);
    if (removedIndex < 0) return;

    qCDebug(configTool) << "syncLayoutDeletedFromFcitx5:" << fcitxKey;
    SyncGuard guard(m_syncInProgress);
    auto updatedEntries = entries;
    updatedEntries.removeAt(removedIndex);
    imConfig->setIMEntries(updatedEntries);
    imConfig->emitChanged();
    imConfig->save();

    if (!updatedEntries.isEmpty()) {
        dbusProvider->controller()->SetCurrentIM(updatedEntries.first().key());
    }
}

void Fcitx5ConfigToolWorkerPrivate::syncFcitx5FirstKeyboardToDCC()
{
    if (!keyboardController || !imConfig) return;

    const auto &entries = imConfig->imEntries();
    QString firstKeyboardLayout;
    for (const auto &entry : entries) {
        QString dccKey = fcitxIMKeyToDccLayout(entry.key());
        if (!dccKey.isEmpty()) {
            firstKeyboardLayout = dccKey;
            break;
        }
    }

    if (firstKeyboardLayout.isEmpty()) return;

    if (!keyboardController->allLayoutsContains(firstKeyboardLayout)) {
        qCDebug(configTool) << "syncFcitx5FirstKeyboardToDCC: layout not in DCC:" << firstKeyboardLayout;
        return;
    }

    qCDebug(configTool) << "syncFcitx5FirstKeyboardToDCC:" << firstKeyboardLayout;
    SyncGuard guard(m_syncInProgress);
    if (!keyboardController->userLayoutsContains(firstKeyboardLayout)) {
        keyboardController->addUserLayout(firstKeyboardLayout);
    }
    keyboardController->setCurrentLayout(firstKeyboardLayout);
}

DCC_FACTORY_CLASS(Fcitx5ConfigToolWorker)
}   // namespace fcitx5configtool
}   // namespace deepin

#include "fcitx5configtool.moc"
