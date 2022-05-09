/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "AdvanceConfig.h"
#include "dbusprovider.h"
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


namespace {
QString joinPath(const QString &path, const QString &option) {
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
{
}

AdvanceConfig::AdvanceConfig(const QMap<QString, FcitxQtConfigOptionList> &desc,
                           QString mainType, DBusProvider *dbus,
                           QObject *parent)
    : QObject(parent)
    , m_desc(desc)
    , m_mainType(mainType)
    , m_dbus(dbus)
{
    m_initialized = true;
}

void AdvanceConfig::requestConfig(bool sync) {
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

void AdvanceConfig::requestConfigFinished(QDBusPendingCallWatcher *watcher) {
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
        setupWidget(nullptr, m_mainType, QString());
        m_initialized = true;
    }

    if (m_initialized) {
        setValue(reply.argumentAt<0>().variant());
    }
}

void AdvanceConfig::load() {
    if (m_uri.isEmpty()) {
        return;
    }
    requestConfig();
}

void AdvanceConfig::save() {
    if (!m_dbus->controller() || m_uri.isEmpty()) {
        return;
    }
    QDBusVariant var(value());
    m_dbus->controller()->SetConfig(m_uri, var);
}

void AdvanceConfig::setValue(const QVariant &value) {
    if (!m_initialized) {
        return;
    }

    m_dontEmitChanged = true;
   // auto optionWidgets = findChildren<OptionWidget *>();
    QVariantMap map;
//    if (value.canConvert<QDBusArgument>()) {
//        auto argument = qvariant_cast<QDBusArgument>(value);
//        argument >> map;
//    } else {
//        map = value.toMap();
//    }
//    for (auto optionWidget : optionWidgets) {
//        optionWidget->readValueFrom(map);
//    }
    m_dontEmitChanged = false;
}

QVariant AdvanceConfig::value() const {
    QVariantMap map;
//    auto optionWidgets = findChildren<OptionWidget *>();
//    for (auto optionWidget : optionWidgets) {
//        optionWidget->writeValueTo(map);
//    }
    return map;
}

void AdvanceConfig::buttonClicked(QDialogButtonBox::StandardButton button) {
//    if (button == QDialogButtonBox::RestoreDefaults) {
//        auto optionWidgets = findChildren<OptionWidget *>();
//        for (auto optionWidget : optionWidgets) {
//            optionWidget->restoreToDefault();
//        }
//    } else if (button == QDialogButtonBox::Ok) {
//        save();
//    }
}

void AdvanceConfig::setupWidget(QWidget *widget, const QString &type,
                               const QString &path) {
    if (!m_desc.contains(type)) {
        qDebug() << type << " type does not exists.";
    }

    auto layout = new QFormLayout(widget);
    const auto &options = m_desc[type];
    for (auto &option : options) {
        addOptionWidget(layout, option, joinPath(path, option.name()));
    }

    widget->setLayout(layout);
}

void AdvanceConfig::addOptionWidget(QFormLayout *layout,
                                   const FcitxQtConfigOption &option,
                                   const QString &path) {
//    if (auto optionWidget =
//            OptionWidget::addWidget(layout, option, path, this)) {
//        connect(optionWidget, &OptionWidget::valueChanged, this,
//                &AdvanceConfig::doChanged);
//    } else if (m_desc.contains(option.type())) {
//        QGroupBox *box = new QGroupBox;
//        box->setTitle(option.description());
//        QVBoxLayout *innerLayout = new QVBoxLayout;
//        QWidget *widget = new QWidget;
//        setupWidget(widget, option.type(), path);
//        innerLayout->addWidget(widget);
//        box->setLayout(innerLayout);
//        layout->addRow(box);
//    } else {
//        qDebug() << "Unknown type: " << option.type();
//    }
}

QDialog *AdvanceConfig::configDialog(QWidget *parent, DBusProvider *dbus,
                                    const QString &uri, const QString &title) {
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

void AdvanceConfig::doChanged() {
    if (m_dontEmitChanged) {
        return;
    }
    emit changed();
}

AdvanceConfig *getConfigWidget(QWidget *widget) {
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
