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
    qDebug() << "Executing command:" << cmd << args;
    QProcess process;
    process.setProgram(cmd);
    process.setArguments(args);

    process.start();
    process.waitForFinished(-1);
    output += process.readAllStandardOutput();
    err += process.readAllStandardError();

    bool success = (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0);
    if (!success) {
        qWarning() << "Command failed:" << cmd << args << "Error:" << err;
    }
    qDebug() << "Command execution result:" << success;
    return success;
}


void ProcessMonitor::checkFcitx5Process() {
    qDebug() << "Checking fcitx5 process status";
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
                qDebug() << "fcitx5 is already running with PID:" << m_previousProcessID;
                return;
            }
        }
        qDebug() << "Clearing invalid previous process ID";
        m_previousProcessID.clear();
    }

    QString output, error;
    exeCommand("pgrep", QStringList() << "-u" << getCurrentUserName() << "-x" << "fcitx5", output, error);
    if (output.isEmpty()) {
        qDebug() << "fcitx5 not running, starting new process";
        m_previousProcessID.clear();
        QProcess process;
        bool started = process.startDetached(PROCESS_NAME, QStringList() << "-d");
        if (started) {
            qDebug() << "Successfully started fcitx5 process";
        } else {
            qWarning() << "Failed to start fcitx5 process";
        }
    } else {
        m_previousProcessID = output.trimmed();
        qDebug() << "Found running fcitx5 process with PID:" << m_previousProcessID;
    }
}

ProcessMonitor::ProcessMonitor(QObject *parent) : QObject(parent)
{
    qDebug() << "Creating ProcessMonitor";
    connect(&m_timer, &QTimer::timeout, this, &ProcessMonitor::checkFcitx5Process);
    qDebug() << "ProcessMonitor created";
}

void ProcessMonitor::startMonitoring() {
    qDebug() << "Starting process monitoring with 4s interval";
    m_timer.start(4000);
    qDebug() << "Process monitoring started";
}
