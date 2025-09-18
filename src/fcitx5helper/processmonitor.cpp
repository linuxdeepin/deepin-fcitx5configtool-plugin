// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "processmonitor.h"

#include <QCoreApplication>
#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(procMon, "fcitx5.helper.processmonitor")

const static QString PROCESS_NAME = "/usr/bin/fcitx5-start";

static QString getCurrentUserName() {
    // qCDebug(procMon) << "Entering getCurrentUserName";
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    return env.value("USER");
}


bool ProcessMonitor::exeCommand(const QString &cmd, const QStringList &args, QString &output, QString &err) {
    qCDebug(procMon) << "Entering exeCommand with command:" << cmd << "and args:" << args;
    QProcess process;
    process.setProgram(cmd);
    process.setArguments(args);

    process.start();
    process.waitForFinished(-1);
    output += process.readAllStandardOutput();
    err += process.readAllStandardError();

    bool success = (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0);
    if (!success) {
        qCWarning(procMon) << "Command failed:" << cmd << args.join(" ") << "Error:" << err;
    }
    qCDebug(procMon) << "Exiting exeCommand with success:" << success;
    return success;
}


void ProcessMonitor::checkFcitx5Process() {
    qCDebug(procMon) << "Entering checkFcitx5Process";
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
                qCDebug(procMon) << "fcitx5 is already running with PID:" << m_previousProcessID;
                return;
            }
        }
        qCDebug(procMon) << "Clearing invalid previous process ID";
        m_previousProcessID.clear();
    }

    QString output, error;
    exeCommand("pgrep", QStringList() << "-u" << getCurrentUserName() << "-x" << "fcitx5", output, error);
    if (output.isEmpty()) {
        qCDebug(procMon) << "fcitx5 not running, starting new process";
        m_previousProcessID.clear();
        QProcess process;
        bool started = process.startDetached(PROCESS_NAME, QStringList() << "-d");
        if (started) {
            qCDebug(procMon) << "Successfully started fcitx5 process";
        } else {
            qCWarning(procMon) << "Failed to start fcitx5 process";
        }
    } else {
        m_previousProcessID = output.trimmed();
        qCDebug(procMon) << "Found running fcitx5 process with PID:" << m_previousProcessID;
    }
    qCDebug(procMon) << "Exiting checkFcitx5Process";
}

ProcessMonitor::ProcessMonitor(QObject *parent) : QObject(parent)
{
    // qCDebug(procMon) << "Entering ProcessMonitor constructor";
    connect(&m_timer, &QTimer::timeout, this, &ProcessMonitor::checkFcitx5Process);
    // qCDebug(procMon) << "Exiting ProcessMonitor constructor";
}

void ProcessMonitor::startMonitoring() {
    // qCDebug(procMon) << "Entering startMonitoring";
    m_timer.start(4000);
    // qCDebug(procMon) << "Exiting startMonitoring";
}
