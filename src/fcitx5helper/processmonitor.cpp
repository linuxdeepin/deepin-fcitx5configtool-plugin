// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "processmonitor.h"

#include <QCoreApplication>
#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>


const static QString PROCESS_NAME = "/usr/bin/fcitx5-start";

static QString getCurrentUserName() {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    return env.value("USER");
}


bool ProcessMonitor::exeCommand(const QString &cmd, const QStringList &args, QString &output, QString &err) {
    QProcess process;
    process.setProgram(cmd);
    process.setArguments(args);

    process.start();
    process.waitForFinished(-1);
    output += process.readAllStandardOutput();
    err += process.readAllStandardError();

    return (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0);
}


void ProcessMonitor::checkFcitx5Process() {
    // Check if the fcitx5 process is already running
    if (!m_previousProcessID.isEmpty() && QFile::exists("/proc/" + m_previousProcessID)) {
        QString commandLine;
        QFile file("/proc/" + m_previousProcessID + "/cmdline");
        if (file.open(QIODevice::ReadOnly)) {
            commandLine = file.readAll().trimmed();
            file.close();
        }
        QStringList arguments = commandLine.split(QRegularExpression("[\\0\\s]+"));
        if (!arguments.isEmpty()) {
            QString executableName = QFileInfo(arguments.first()).fileName();
            if (executableName == "fcitx5" || arguments.first() == "fcitx5") {
                return; // fcitx5 is already running
            }
        }
        m_previousProcessID.clear();
    }

    QString output, error;
    exeCommand("pgrep", QStringList() << "-u" << getCurrentUserName() << "-x" << "fcitx5", output, error);
    if (output.isEmpty()) {
        m_previousProcessID.clear();
        QProcess process;
        process.startDetached(PROCESS_NAME, QStringList() << "-d");
    } else {
        m_previousProcessID = output.trimmed();
    }
}

ProcessMonitor::ProcessMonitor(QObject *parent) : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &ProcessMonitor::checkFcitx5Process);
}

void ProcessMonitor::startMonitoring() {
    m_timer.start(4000);
}
