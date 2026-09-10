// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "keyboardmonitor.h"

#include <libudev.h>

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QDBusVariant>
#include <QLoggingCategory>
#include <QSocketNotifier>
#include <QTimer>

Q_LOGGING_CATEGORY(keyboardMon, "fcitx5.helper.keyboardmonitor")

namespace {
constexpr char FcitxService[] = "org.fcitx.Fcitx5";
constexpr char ControllerPath[] = "/controller";
constexpr char ControllerInterface[] = "org.fcitx.Fcitx.Controller1";
constexpr char VirtualKeyboardConfigUri[] =
    "fcitx://config/addon/virtualkeyboard";
constexpr int RetryIntervalMs = 1000;
constexpr int DebounceIntervalMs = 100;
constexpr int DBusTimeoutMs = 3000;
constexpr int MaxRetryCount = 30;
constexpr char VirtualKeyboardService[] = "org.fcitx.Fcitx5.VirtualKeyboard";
constexpr char VirtualKeyboardPath[] = "/virtualkeyboard";
constexpr char VirtualKeyboardInterface[] = "org.fcitx.Fcitx.VirtualKeyboard1";
} // namespace

KeyboardMonitor::KeyboardMonitor(QObject *parent) : QObject(parent) {
    m_retryTimer = new QTimer(this);
    m_retryTimer->setInterval(RetryIntervalMs);
    connect(m_retryTimer, &QTimer::timeout, this, [this]() {
        if (m_retryCount >= MaxRetryCount) {
            qCWarning(keyboardMon) << "Max retry count (" << MaxRetryCount
                                   << ") reached; stopping retry";
            m_retryTimer->stop();
            m_retryCount = 0;
            return;
        }
        ++m_retryCount;
        requestVirtualKeyboard(m_pendingEnable);
    });

    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(DebounceIntervalMs);
    connect(m_debounceTimer, &QTimer::timeout, this,
            [this]() { performKeyboardCheck(); });

    // Recover when Fcitx5 (re)appears on the bus: the bounded retry loop may
    // already have given up, and the option-check latch must be re-evaluated
    // against the new Fcitx5 instance.
    auto *serviceWatcher = new QDBusServiceWatcher(
        QString::fromLatin1(FcitxService), QDBusConnection::sessionBus(),
        QDBusServiceWatcher::WatchForRegistration, this);
    connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this,
            &KeyboardMonitor::onFcitxServiceRegistered);
}

KeyboardMonitor::~KeyboardMonitor() {
    m_debounceTimer->stop();
    m_retryTimer->stop();
    if (m_monitor) {
        udev_monitor_unref(m_monitor);
    }
    if (m_udev) {
        udev_unref(m_udev);
    }
}

bool KeyboardMonitor::start() {
    if (m_started) {
        return true;
    }

    // Clean up any resources from a previous failed attempt to avoid leaks.
    if (m_monitor) {
        udev_monitor_unref(m_monitor);
        m_monitor = nullptr;
    }
    if (m_udev) {
        udev_unref(m_udev);
        m_udev = nullptr;
    }
    if (m_notifier) {
        delete m_notifier;
        m_notifier = nullptr;
    }

    m_udev = udev_new();
    if (!m_udev) {
        qCWarning(keyboardMon) << "Failed to create udev context";
        return false;
    }

    m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
    if (!m_monitor) {
        qCWarning(keyboardMon) << "Failed to create udev monitor";
        udev_unref(m_udev);
        m_udev = nullptr;
        return false;
    }

    if (udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "input",
                                                          nullptr) < 0 ||
        udev_monitor_enable_receiving(m_monitor) < 0) {
        qCWarning(keyboardMon) << "Failed to configure udev monitor";
        udev_monitor_unref(m_monitor);
        m_monitor = nullptr;
        udev_unref(m_udev);
        m_udev = nullptr;
        return false;
    }

    const int fd = udev_monitor_get_fd(m_monitor);
    if (fd < 0) {
        qCWarning(keyboardMon) << "Failed to get udev monitor file descriptor";
        udev_monitor_unref(m_monitor);
        m_monitor = nullptr;
        udev_unref(m_udev);
        m_udev = nullptr;
        return false;
    }

    m_notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this,
            [this](QSocketDescriptor) { processUdevEvents(); });

    m_started = true;
    // Enumeration failure defaults to "no keyboard" deliberately: enabling
    // the virtual keyboard is the safe failure direction (a physical
    // keyboard keeps working with the addon enabled, whereas a touch-only
    // device that lost the virtual keyboard would have no input at all).
    updateKeyboardState(enumerateKeyboard().value_or(false), true);
    qCInfo(keyboardMon) << "Keyboard monitoring started; keyboard present:"
                        << m_hasKeyboard;
    return true;
}

void KeyboardMonitor::queryVirtualKeyboardOption() {
    if (m_optionQueryPending) {
        // A query is already in flight; its reply handler continues the flow.
        return;
    }
    m_optionQueryPending = true;

    QDBusMessage msg = QDBusMessage::createMethodCall(
        QString::fromLatin1(FcitxService), QString::fromLatin1(ControllerPath),
        QString::fromLatin1(ControllerInterface), QStringLiteral("GetConfig"));
    msg << QString::fromLatin1(VirtualKeyboardConfigUri);
    auto *watcher = new QDBusPendingCallWatcher(
        QDBusConnection::sessionBus().asyncCall(msg, DBusTimeoutMs), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, watcher]() {
                watcher->deleteLater();
                handleOptionReply(watcher);
            });
}

void KeyboardMonitor::handleOptionReply(QDBusPendingCallWatcher *watcher) {
    m_optionQueryPending = false;

    // The reply type is a variant whose payload is the addon RawConfig
    // (`a{sv}` of option name to string value).
    const QDBusPendingReply<QDBusVariant> reply = *watcher;
    if (reply.isError()) {
        const QString errorName = reply.error().name();
        qCWarning(keyboardMon)
            << "Failed to query virtual keyboard addon configuration:"
            << errorName << reply.error().message();

        // Fcitx5 may not have started yet. Keep retrying in that case. An
        // existing addon without a configurable EnableVirtualKeyboard option
        // is a permanent compatibility case and must not be retried forever.
        if (errorName == QStringLiteral("org.freedesktop.DBus.Error.ServiceUnknown") ||
            errorName == QStringLiteral("org.freedesktop.DBus.Error.NoReply")) {
            m_retryTimer->start();
        } else {
            m_optionChecked = true;
            m_retryCount = 0;
            m_retryTimer->stop();
        }
        return;
    }

    QVariant value = reply.value().variant();
    QVariantMap config;
    if (value.canConvert<QDBusArgument>()) {
        const auto argument = qvariant_cast<QDBusArgument>(value);
        argument >> config;
    } else {
        config = value.toMap();
    }

    m_optionChecked = true;
    m_retryCount = 0;
    m_retryTimer->stop();
    m_optionAvailable = config.contains(QStringLiteral("EnableVirtualKeyboard"));
    m_currentEnableState = -1;
    if (!m_optionAvailable) {
        qCWarning(keyboardMon)
            << "Virtual keyboard addon does not provide"
            << "EnableVirtualKeyboard; hardware monitoring will not change"
            << "the addon configuration";
        return;
    }

    QVariant optionValue = config.value(QStringLiteral("EnableVirtualKeyboard"));
    if (optionValue.canConvert<QDBusVariant>()) {
        optionValue = qvariant_cast<QDBusVariant>(optionValue).variant();
    }
    const QString text = optionValue.toString();
    if (text.compare(QStringLiteral("True"), Qt::CaseInsensitive) == 0) {
        m_currentEnableState = 1;
    } else if (text.compare(QStringLiteral("False"), Qt::CaseInsensitive) == 0) {
        m_currentEnableState = 0;
    }
    applyVirtualKeyboardOption();
}

std::optional<bool> KeyboardMonitor::enumerateKeyboard() const {
    udev_enumerate *enumerate = udev_enumerate_new(m_udev);
    if (!enumerate) {
        qCWarning(keyboardMon) << "Failed to create udev enumerator";
        return std::nullopt;
    }

    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_add_match_property(enumerate, "ID_INPUT_KEYBOARD", "1");
    udev_enumerate_scan_devices(enumerate);

    bool hasKeyboard = false;
    udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    udev_list_entry *entry = nullptr;
    udev_list_entry_foreach(entry, devices) {
        hasKeyboard = true;
        break;
    }

    udev_enumerate_unref(enumerate);
    return hasKeyboard;
}

void KeyboardMonitor::onFcitxServiceRegistered() {
    if (!m_started) {
        return;
    }
    qCInfo(keyboardMon)
        << "Fcitx5 service registered; re-applying virtual keyboard state";
    // Reset the option-check latch: the (re)started Fcitx5 instance may
    // expose a different addon configuration than the previous one.
    m_optionChecked = false;
    m_optionAvailable = false;
    m_currentEnableState = -1;
    m_retryCount = 0;
    m_retryTimer->stop();
    requestVirtualKeyboard(m_pendingEnable);
}

void KeyboardMonitor::processUdevEvents() {
    bool inputChanged = false;
    int eventCount = 0;
    while (auto *device = udev_monitor_receive_device(m_monitor)) {
        const char *isKeyboard =
            udev_device_get_property_value(device, "ID_INPUT_KEYBOARD");
        qCInfo(keyboardMon)
            << "Received udev input event:" << udev_device_get_action(device)
            << udev_device_get_sysname(device)
            << "ID_INPUT_KEYBOARD=" << (isKeyboard ? isKeyboard : "(null)");
        if (isKeyboard && isKeyboard[0] == '1') {
            inputChanged = true;
            ++eventCount;
        }
        udev_device_unref(device);
    }

    if (inputChanged) {
        qCInfo(keyboardMon) << "Processed" << eventCount
                            << "udev input event(s); debouncing device"
                            << "enumeration";
        m_debounceTimer->start();
    }
}

void KeyboardMonitor::performKeyboardCheck() {
    const std::optional<bool> hasKeyboard = enumerateKeyboard();
    if (!hasKeyboard) {
        // Enumeration failure must not be mistaken for "no keyboard";
        // keep the last known state.
        qCWarning(keyboardMon)
            << "Keyboard enumeration failed; keeping last known state:"
            << m_hasKeyboard;
        return;
    }
    qCInfo(keyboardMon) << "Debounce timer elapsed; keyboard present:"
                        << *hasKeyboard;
    updateKeyboardState(*hasKeyboard);
}

void KeyboardMonitor::updateKeyboardState(bool hasKeyboard, bool force) {
    if (!force && m_hasKeyboard == hasKeyboard) {
        qCInfo(keyboardMon)
            << "Keyboard state unchanged; keyboard present:" << hasKeyboard;
        return;
    }

    qCInfo(keyboardMon) << "Keyboard state changed from" << m_hasKeyboard
                        << "to" << hasKeyboard;
    m_hasKeyboard = hasKeyboard;
    requestVirtualKeyboard(!m_hasKeyboard);
}

void KeyboardMonitor::requestVirtualKeyboard(bool enableVirtualKeyboard) {
    if (m_pendingEnable != enableVirtualKeyboard) {
        m_retryCount = 0;
    }
    m_pendingEnable = enableVirtualKeyboard;

    if (!m_optionChecked) {
        // Fetch the addon option asynchronously; the reply handler continues
        // with applyVirtualKeyboardOption().
        queryVirtualKeyboardOption();
        return;
    }
    if (!m_optionAvailable) {
        return;
    }
    applyVirtualKeyboardOption();
}

void KeyboardMonitor::applyVirtualKeyboardOption() {
    const int targetState = m_pendingEnable ? 1 : 0;
    if (m_currentEnableState == targetState) {
        // Already in the desired state. Skip the SetConfig write so a normal
        // login does not rewrite persistent config (which would clobber a
        // value the user changed manually elsewhere).
        qCInfo(keyboardMon)
            << "Virtual keyboard addon already"
            << (m_pendingEnable ? "enabled" : "disabled")
            << "; skipping SetConfig";
        m_retryCount = 0;
        m_retryTimer->stop();
        return;
    }

    if (!m_pendingEnable) {
        // Must run BEFORE SetConfig: writing EnableVirtualKeyboard makes the
        // addon reconfigure itself and silently drops facade hide calls issued
        // inside that window.
        hideVirtualKeyboard();
    }

    // Record the target optimistically so a state flip arriving while the
    // call is in flight is still detected as a change; reverted on error.
    m_currentEnableState = targetState;

    // Change the virtualkeyboard addon configuration through Fcitx5's
    // standard controller API. The outer QDBusVariant is the `v` argument of
    // SetConfig; the inner map is the addon RawConfig (`a{sv}`).
    QVariantMap config;
    config.insert(
        QStringLiteral("EnableVirtualKeyboard"),
        m_pendingEnable ? QStringLiteral("True") : QStringLiteral("False"));
    const QDBusVariant value(config);
    QDBusMessage msg = QDBusMessage::createMethodCall(
        QString::fromLatin1(FcitxService), QString::fromLatin1(ControllerPath),
        QString::fromLatin1(ControllerInterface), QStringLiteral("SetConfig"));
    msg << QString::fromLatin1(VirtualKeyboardConfigUri)
        << QVariant::fromValue(value);
    const bool enable = m_pendingEnable;
    auto *watcher = new QDBusPendingCallWatcher(
        QDBusConnection::sessionBus().asyncCall(msg, DBusTimeoutMs), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, watcher, enable]() {
                watcher->deleteLater();
                handleSetConfigReply(watcher, enable);
            });
}

void KeyboardMonitor::handleSetConfigReply(QDBusPendingCallWatcher *watcher,
                                            bool enableVirtualKeyboard) {
    const QDBusPendingReply<> reply = *watcher;
    if (reply.isError()) {
        // Revert the optimistic state so the next attempt re-issues the
        // write, unless a newer SetConfig already superseded this one and
        // its optimistic value must survive.
        if ((enableVirtualKeyboard ? 1 : 0) == m_currentEnableState) {
            m_currentEnableState = -1;
        }
        const QString errorName = reply.error().name();
        qCWarning(keyboardMon)
            << "Failed to set virtual keyboard addon configuration to"
            << enableVirtualKeyboard
            << ":" << errorName << reply.error().message();

        // Only retry on transient errors; permanent failures must not loop
        // forever. This mirrors the logic in handleOptionReply().
        if (errorName == QStringLiteral("org.freedesktop.DBus.Error.ServiceUnknown") ||
            errorName == QStringLiteral("org.freedesktop.DBus.Error.NoReply")) {
            m_retryTimer->start();
        } else {
            m_retryCount = 0;
            m_retryTimer->stop();
        }
        return;
    }

    m_retryCount = 0;
    m_retryTimer->stop();
    qCInfo(keyboardMon)
        << "Virtual keyboard addon configuration set to"
        << enableVirtualKeyboard
        << (enableVirtualKeyboard ? "(enabled)" : "(disabled)");
}

void KeyboardMonitor::hideVirtualKeyboard() {
    // Fire-and-forget: the virtual keyboard process runs on demand only, so
    // the service may be absent when nothing is shown. Any error here must
    // not feed back into the retry logic.
    const QDBusMessage message = QDBusMessage::createMethodCall(
        QString::fromLatin1(VirtualKeyboardService),
        QString::fromLatin1(VirtualKeyboardPath),
        QString::fromLatin1(VirtualKeyboardInterface),
        QStringLiteral("HideVirtualKeyboard"));
    QDBusConnection::sessionBus().asyncCall(message);
}
