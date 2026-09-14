// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef KEYBOARDMONITOR_H
#define KEYBOARDMONITOR_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

#include <optional>
#include <utility>

class QDBusPendingCallWatcher;
class QSocketNotifier;
class QTimer;
struct udev;
struct udev_monitor;

class KeyboardMonitor : public QObject 
{
    Q_OBJECT

public:
    struct DeviceInfo {
        QString name;
        QString handlers;
        QString physPath;
        QString sysfsPath;
        QString eventCapabilities;
    };

    using DeviceInfoMap = QMap<QString, QList<DeviceInfo>>;

    explicit KeyboardMonitor(QObject *parent = nullptr);
    ~KeyboardMonitor() override;

    bool start();

private:
    std::optional<bool> enumerateKeyboard() const;
    static void addDeviceToMap(DeviceInfoMap &deviceInfoMap,
                               const QString &id, const DeviceInfo &info);
    static void parseAllDeviceBlocks(const QList<QStringList> &blocks,
                                     DeviceInfoMap &deviceInfoMap);
    static QList<QStringList> splitIntoDeviceBlocks(
        const QString &content);
    static std::pair<QString, DeviceInfo>
    parseSingleDeviceBlock(const QStringList &block);
    static void parseDeviceLine(const QString &line, QString &id,
                                DeviceInfo &info);
    static bool hasKeyboard(const DeviceInfoMap &deviceInfoMap);
    void queryVirtualKeyboardOption();
    void handleOptionReply(QDBusPendingCallWatcher *watcher);
    void applyVirtualKeyboardOption();
    void handleSetConfigReply(QDBusPendingCallWatcher *watcher,
                              bool enableVirtualKeyboard, int requestId);
    void processUdevEvents();
    void performKeyboardCheck();
    void updateKeyboardState(bool hasKeyboard, bool force = false);
    void requestVirtualKeyboard(bool enableVirtualKeyboard);
    void hideVirtualKeyboard();
    void onFcitxServiceRegistered();

    udev *m_udev = nullptr;
    udev_monitor *m_monitor = nullptr;
    QSocketNotifier *m_notifier = nullptr;
    QTimer *m_retryTimer = nullptr;
    QTimer *m_debounceTimer = nullptr;
    bool m_hasKeyboard = false;
    bool m_pendingEnable = false;
    bool m_optionChecked = false;
    bool m_optionQueryPending = false;
    bool m_optionAvailable = false;
    // -1 unknown, 0 disabled, 1 enabled (current persisted addon option).
    int m_currentEnableState = -1;
    bool m_started = false;
    int m_retryCount = 0;
    int m_configRequestId = 0;
    bool m_keyboardStateKnown = false;
};

#endif // KEYBOARDMONITOR_H
