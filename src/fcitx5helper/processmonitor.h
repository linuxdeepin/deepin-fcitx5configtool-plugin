// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef PROCESSMONITOR_H
#define PROCESSMONITOR_H

#include <QObject>
#include <QTimer>
#include <QString>
class ProcessMonitor : public QObject 
{
    Q_OBJECT

public:
    explicit ProcessMonitor(QObject *parent = nullptr);
    void startMonitoring();

    static bool exeCommand(const QString &cmd, const QStringList &args, QString &output, QString &err);

private Q_SLOTS:
    void checkFcitx5Process();

private:
    QTimer m_timer;
    QString m_previousProcessID;
};

#endif // PROCESSMONITOR_H
