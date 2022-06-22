/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "advanceconfig.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <fcitx-utils/i18n.h>
#include <fcitxqtcontrollerproxy.h>

#include "varianthelper.h"
#include "dbusprovider.h"
#include "configwidgetslib/keylistwidget.h"
#include "configwidgetslib/optionwidget.h"
#include "publisher/publisherfunc.h"

namespace {
QString joinPath(const QString &path, const QString &option)
{
    if (path.isEmpty()) {
        return option;
    }
    return QString("%1/%2").arg(path, option);
}
} // namespace

AdvanceConfig::AdvanceConfig(const QString &uri, DBusProvider *dbus, QObject *parent)
    : QObject(parent)
    , m_uri(uri)
    , m_dbus(dbus)
    , m_configWidget(new QWidget)
{
    requestConfig();
}

void AdvanceConfig::requestConfig(bool sync)
{
    if (!m_dbus->controller()) {
        return;
    }
    auto call = m_dbus->controller()->GetConfig(m_uri);
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            &AdvanceConfig::requestConfigFinished);
    if (sync) {
        watcher->waitForFinished();
    }
}

void AdvanceConfig::requestConfigFinished(QDBusPendingCallWatcher *watcher)
{
    watcher->deleteLater();
    QDBusPendingReply<QDBusVariant, FcitxQtConfigTypeList> reply = *watcher;
    if (reply.isError()) {
        return;
    }

    // qCDebug(KCM_FCITX5) << reply.argumentAt<0>().variant();

    if (!m_initialized) {
        auto desc = reply.argumentAt<1>();
        if (!desc.size()) {
            return;
        }

        for (auto &type : desc) {
            m_desc[type.name()] = type.options();
        }
        m_mainType = desc[0].name();
        setupData(m_mainType, QString());
        m_initialized = true;
    }

    if (m_initialized) {
        setValue(reply.argumentAt<0>().variant());
    }
}

void AdvanceConfig::load()
{
    if (m_uri.isEmpty()) {
        return;
    }
    requestConfig();
}

void AdvanceConfig::save()
{
    if (!m_dbus->controller() || m_uri.isEmpty()) {
        return;
    }
    QDBusVariant var(value());
    m_dbus->controller()->SetConfig(m_uri, var);
}

void AdvanceConfig::setValue(const QVariant &value)
{
    if (!m_initialized) {
        return;
    }

    m_dontEmitChanged = true;
    auto optionWidgets = m_configWidget->findChildren<OptionWidget *>();
    QVariantMap map;
    if (value.canConvert<QDBusArgument>()) {
        auto argument = qvariant_cast<QDBusArgument>(value);
        argument >> map;
    } else {
        map = value.toMap();
    }
    QList<fcitx::Key> keyList1 = readValue(map, "Hotkey/TriggerKeys");
    QList<fcitx::Key> keyList2 = readValue(map, "Hotkey/EnumerateForwardKeys");
    QString s1 = QString::fromUtf8(Key::keyListToString(keyList1, KeyStringFormat::Portable).c_str());
    QString s2 = QString::fromUtf8(Key::keyListToString(keyList2, KeyStringFormat::Portable).c_str());
    emit switchIMShortCutsChanged(s2);
    emit switchFirstIMShortCutsChanged(s1);

    for (auto optionWidget : optionWidgets) {
        optionWidget->readValueFrom(map);
    }
    m_dontEmitChanged = false;
}

QVariant AdvanceConfig::value() const
{
    QVariantMap map;
    auto optionWidgets = m_configWidget->findChildren<OptionWidget *>();
    for (auto optionWidget : optionWidgets) {
        optionWidget->writeValueTo(map);
    }
    if(!m_switchIMShortCuts.isEmpty()) {
        writeVariant(map, QString("Hotkey/EnumerateForwardKeys/0"), m_switchIMShortCuts.split(" ").first());
        writeVariant(map, QString("Hotkey/EnumerateForwardKeys/1"), m_switchIMShortCuts.split(" ").last());
    }
    if(!m_switchFirstIMShortCuts.isEmpty()) {
        writeVariant(map, "Hotkey/TriggerKeys/0", m_switchFirstIMShortCuts);
    }

    return map;
}

void AdvanceConfig::buttonClicked(QDialogButtonBox::StandardButton button)
{
    if (button == QDialogButtonBox::RestoreDefaults) {
//        auto optionWidgets = findChildren<OptionWidget *>();
//        for (auto optionWidget : optionWidgets) {
//            optionWidget->restoreToDefault();
//        }
    } else if (button == QDialogButtonBox::Ok) {
        save();
    }
}

void AdvanceConfig::setupData(const QString &type, const QString &path)
{
    if (!m_desc.contains(type)) {
        qDebug() << type << " type does not exists.";
    }

    const auto &options = m_desc[type];
    for (auto &option : options) {
        addChildData(option, joinPath(path, option.name()));
    }
}

void AdvanceConfig::addChildData(const FcitxQtConfigOption &option, const QString &path)
{
    QFormLayout *layout = new QFormLayout;
    if (auto optionWidget =
            OptionWidget::addWidget(layout, option, path, m_configWidget)) {
        connect(optionWidget, &OptionWidget::valueChanged, this,
                &AdvanceConfig::doChanged);
    } else if (m_desc.contains(option.type())) {
        QGroupBox *box = new QGroupBox;
        box->setTitle(option.description());
        QVBoxLayout *innerLayout = new QVBoxLayout;
        QWidget *widget = new QWidget;
        setupData(option.type(), path);
        innerLayout->addWidget(widget);
        box->setLayout(innerLayout);
        layout->addRow(box);
    } else {
        qDebug() << "Unknown type: " << option.type();
    }
}

QList<fcitx::Key> AdvanceConfig::readValue(const QVariantMap &map, const QString &path)
{
    int i = 0;
    QList<Key> keys;
    while (true) {
        auto value = readString(map, QString("%1%2%3")
                                         .arg(path)
                                         .arg(path.isEmpty() ? "" : "/")
                                         .arg(i));
        if (value.isNull()) {
            break;
        }
        keys << Key(value.toUtf8().constData());
        i++;
    }
    return keys;
}

QDialog *AdvanceConfig::configDialog(QWidget *parent, DBusProvider *dbus,
                                    const QString &uri, const QString &title)
{
    auto configPage = new AdvanceConfig(uri, dbus);
    configPage->requestConfig(true);
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
                             QDialogButtonBox::RestoreDefaults);

    buttonBox->button(QDialogButtonBox::Ok)->setText(_("&OK"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(_("&Cancel"));
    buttonBox->button(QDialogButtonBox::RestoreDefaults)
        ->setText(_("Restore &Defaults"));

//    auto configPageWrapper = new VerticalScrollArea;
//    configPageWrapper->setWidget(configPage);
//    dialogLayout->addWidget(configPageWrapper);
    dialogLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::clicked, configPage,
            [configPage, buttonBox](QAbstractButton *button) {
                configPage->buttonClicked(buttonBox->standardButton(button));
            });

    QDialog *dialog = new QDialog(parent);
    dialog->setWindowIcon(QIcon::fromTheme("fcitx"));
    dialog->setWindowTitle(title);
    dialog->setLayout(dialogLayout);
    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    return dialog;
}

void AdvanceConfig::switchIMShortCuts(const QString &shortCuts)
{
    if(shortCuts == "CTRL_SHIFT") {
        m_switchIMShortCuts = "Control+Shift+Shift_L Control+Shift+Shift_R";
    } else if(shortCuts == "ALT_SHIFT") {
        m_switchIMShortCuts = "Alt+Shift+Shift_R Alt+Shift+Shift_L";
    } else if(shortCuts == "CTRL_SUPER") {
        m_switchIMShortCuts = "Control+Super+Control_L Control+Super+Control_R";
    } else if(shortCuts == "ALT_SUPER") {
        m_switchIMShortCuts = "Alt+Super+Alt_L Alt+Super+Alt_R";
    }
    KeyList list = Key::keyListFromString(m_switchIMShortCuts.toStdString());
    QList<fcitx::Key> klist;
    for(std::vector<fcitx::Key>::iterator it = list.begin(); it != list.end(); it++)
    {
        klist << *it;
    }
    save();
    qDebug() << "finished";
}

void AdvanceConfig::switchFirstIMShortCuts(const QString &shortCuts)
{
    m_switchFirstIMShortCuts = publisherFunc::transFirstUpper(shortCuts);
    KeyList list = Key::keyListFromString(m_switchFirstIMShortCuts.toStdString());
    QList<fcitx::Key> klist;
    for(std::vector<fcitx::Key>::iterator it = list.begin(); it != list.end(); it++)
    {
        klist << *it;
    }
    save();
    qDebug() << "finished";
}

void AdvanceConfig::doChanged()
{
    if (m_dontEmitChanged) {
        return;
    }
    emit changed();
}

AdvanceConfig *getConfigWidget(QWidget *widget)
{
    widget = widget->parentWidget();
    AdvanceConfig *configWidget;
    while (widget) {
        configWidget = qobject_cast<AdvanceConfig *>(widget);
        if (configWidget) {
            break;
        }
        widget = widget->parentWidget();
    }
    return configWidget;
}
