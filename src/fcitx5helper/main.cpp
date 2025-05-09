// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "processmonitor.h"

#include <QCoreApplication>

#include <signal.h>

static void signal_callback_handler(int signum, siginfo_t *siginfo, void *context) {
    Q_UNUSED(context)
    if (signum == SIGTERM && (long)siginfo->si_pid == 1) {
        QString output, error;
        ProcessMonitor::exeCommand("pidof fcitx5 | xargs kill -9", QStringList(), output, error);
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
    QCoreApplication app(argc, argv);

    setupSignalHandler();
    
    ProcessMonitor monitor;
    monitor.startMonitoring();

    app.exec();
}
