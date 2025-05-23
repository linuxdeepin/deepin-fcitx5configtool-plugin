// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "processmonitor.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(fcitx5Helper, "fcitx5.helper")

#include <QCoreApplication>

#include <signal.h>

static void signal_callback_handler(int signum, siginfo_t *siginfo, void *context) {
    Q_UNUSED(context)
    qCDebug(fcitx5Helper) << "Received signal:" << signum << "from pid:" << (long)siginfo->si_pid;
    if (signum == SIGTERM && (long)siginfo->si_pid == 1) {
        qCInfo(fcitx5Helper) << "Terminating fcitx5 processes due to SIGTERM from init";
        QString output, error;
        ProcessMonitor::exeCommand("pidof fcitx5 | xargs kill -9", QStringList(), output, error);
        qCDebug(fcitx5Helper) << "Command output:" << output << "Error:" << error;
        exit(signum);
    }
}

static void setupSignalHandler() {
    //进城被init杀死，视为关机
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = &signal_callback_handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &act, nullptr);
}

int main(int argc, char *argv[])
{
    qCInfo(fcitx5Helper) << "Starting fcitx5 helper process";
    QCoreApplication app(argc, argv);

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
