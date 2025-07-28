// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "processmonitor.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(fcitx5Helper, "fcitx5.helper")

#include <QCoreApplication>
#include <QDir>
#include <QLockFile>
#include <QScopedPointer>
#include <unistd.h>

#include <signal.h>

static QScopedPointer<QLockFile> g_lockFile;

// Extract from DGuiApplicationHelper::setSingleInstance (UserScope only)
static bool setSingleInstance() {
    QString socket_key = "_fcitx5_single_instance_";
    socket_key += QString("%1_").arg(getuid());
    socket_key += QString("fcitx5-helper");

    QString lockfile = socket_key;
    if (!lockfile.startsWith(QLatin1Char('/'))) {
        lockfile = QDir::cleanPath(QDir::tempPath());
        lockfile += QLatin1Char('/') + socket_key;
        qCDebug(fcitx5Helper) << "lockfile:" << lockfile;
    }
    lockfile += QStringLiteral(".lock");

    qint64 pid = -1;
    QString hostname, appname;
    if (!g_lockFile.isNull() && g_lockFile->isLocked() && g_lockFile->getLockInfo(&pid, &hostname, &appname) && pid == getpid()) {
        qCWarning(fcitx5Helper) << "call setSingleInstance again within the same process";
        g_lockFile->unlock();
        g_lockFile.reset();
        qCDebug(fcitx5Helper) << "unlock lock file";
    }

    // Create new lock file
    g_lockFile.reset(new QLockFile(lockfile));

    // Try to lock
    if (!g_lockFile->tryLock()) {
        qCInfo(fcitx5Helper) << "fcitx5-helper instance already exists for current user, exiting";
        return false;
    }

    qCDebug(fcitx5Helper) << "Created lock file:" << lockfile;
    return true;
}

static void signal_callback_handler(int signum, siginfo_t *siginfo, void *context) {
    Q_UNUSED(context)
    qCDebug(fcitx5Helper) << "Received signal:" << signum << "from pid:" << (long)siginfo->si_pid;
    if (signum == SIGTERM && (long)siginfo->si_pid == 1) {
        qCInfo(fcitx5Helper) << "Terminating fcitx5 processes due to SIGTERM from init";
        // Clean up lock file
        if (!g_lockFile.isNull() && g_lockFile->isLocked()) {
            qCInfo(fcitx5Helper) << "unlock lock file";
            g_lockFile->unlock();
        }
        QString output, error;
        ProcessMonitor::exeCommand("pidof fcitx5 | xargs kill -9", QStringList(), output, error);
        qCDebug(fcitx5Helper) << "Command output:" << output << "Error:" << error;
        exit(signum);
    }
}

static void setupSignalHandler() {
    //进程被init杀死，视为关机
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = &signal_callback_handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &act, nullptr);
}

int main(int argc, char *argv[])
{
    qCInfo(fcitx5Helper) << "Starting fcitx5 helper process";
    // Check single instance for current user
    if (!setSingleInstance()) {
        qCInfo(fcitx5Helper) << "fcitx5-helper already running";
        return 0;
    }
    QCoreApplication app(argc, argv);

    // Register cleanup function to unlock on exit
    QObject::connect(&app, &QCoreApplication::aboutToQuit, []() {
        if (!g_lockFile.isNull() && g_lockFile->isLocked()) {
            qCInfo(fcitx5Helper) << "unlock lock file";
            g_lockFile->unlock();
        }
    });

    qCDebug(fcitx5Helper) << "Setting up signal handlers";
    setupSignalHandler();

    qCDebug(fcitx5Helper) << "Initializing process monitor";
    ProcessMonitor monitor;
    monitor.startMonitoring();
    qCInfo(fcitx5Helper) << "Process monitoring started";

    int ret = app.exec();
    qCDebug(fcitx5Helper) << "Application exiting with code:" << ret;
    return ret;
}
