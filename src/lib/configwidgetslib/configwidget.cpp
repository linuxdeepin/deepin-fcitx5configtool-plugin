/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "configwidget.h"
#include "dbusprovider.h"
#include "keylistwidget.h"
#include "logging.h"
#include "optionwidget.h"
#include "verticalscrollarea.h"
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

namespace fcitx {
namespace kcm {

namespace {
QString joinPath(const QString &path, const QString &option) {
    if (path.isEmpty()) {
        return option;
    }
    return QString("%1/%2").arg(path, option);
}
} // namespace

ConfigWidget::ConfigWidget(const QString &uri, DBusProvider *dbus,
                           QWidget *parent)
    : QWidget(parent), uri_(uri), dbus_(dbus), mainWidget_(new QWidget(this)) {
    qCDebug(KCM_FCITX5) << "ConfigWidget created with uri:" << uri;
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mainWidget_);
    setLayout(layout);
    qCDebug(KCM_FCITX5) << "Exiting ConfigWidget constructor";
}

ConfigWidget::ConfigWidget(const QMap<QString, FcitxQtConfigOptionList> &desc,
                           QString mainType, DBusProvider *dbus,
                           QWidget *parent)
    : QWidget(parent), desc_(desc), mainType_(mainType), dbus_(dbus),
      mainWidget_(new QWidget(this)) {
    qCDebug(KCM_FCITX5) << "Entering ConfigWidget constructor with description for main type:" << mainType;
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mainWidget_);
    setLayout(layout);

    setupWidget(mainWidget_, mainType_, QString());
    initialized_ = true;
    qCDebug(KCM_FCITX5) << "Exiting ConfigWidget constructor";
}

void ConfigWidget::requestConfig(bool sync) {
    qCDebug(KCM_FCITX5) << "Requesting config for uri:" << uri_ << "sync:" << sync;
    if (!dbus_->controller()) {
        qCWarning(KCM_FCITX5) << "DBus controller not available";
        return;
    }
    auto call = dbus_->controller()->GetConfig(uri_);
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            &ConfigWidget::requestConfigFinished);
    if (sync) {
        qCDebug(KCM_FCITX5) << "Waiting for config request to finish";
        watcher->waitForFinished();
    }
    qCDebug(KCM_FCITX5) << "Exiting requestConfig";
}

void ConfigWidget::requestConfigFinished(QDBusPendingCallWatcher *watcher) {
    qCDebug(KCM_FCITX5) << "Config request finished";
    watcher->deleteLater();
    QDBusPendingReply<QDBusVariant, FcitxQtConfigTypeList> reply = *watcher;
    if (reply.isError()) {
        qCWarning(KCM_FCITX5) << "Config request error:" << reply.error();
        return;
    }

    // qCDebug(KCM_FCITX5) << reply.argumentAt<0>().variant();

    if (!initialized_) {
        qCInfo(KCM_FCITX5) << "Widget not initialized, setting up from received description.";
        auto desc = reply.argumentAt<1>();
        if (!desc.size()) {
            qCWarning(KCM_FCITX5) << "Received empty description, aborting setup.";
            return;
        }

        for (auto &type : desc) {
            desc_[type.name()] = type.options();
        }
        mainType_ = desc[0].name();
        setupWidget(mainWidget_, mainType_, QString());
        initialized_ = true;
    }

    if (initialized_) {
        qCDebug(KCM_FCITX5) << "Setting value from received config";
        setValue(reply.argumentAt<0>().variant());
    }

    adjustSize();
    qCDebug(KCM_FCITX5) << "Exiting requestConfigFinished";
}

void ConfigWidget::load() {
    qCDebug(KCM_FCITX5) << "Entering load";
    if (uri_.isEmpty()) {
        qCWarning(KCM_FCITX5) << "Cannot load config: URI is empty.";
        return;
    }
    requestConfig();
    qCDebug(KCM_FCITX5) << "Exiting load";
}

void ConfigWidget::save() {
    qCDebug(KCM_FCITX5) << "Saving config";
    if (!dbus_->controller() || uri_.isEmpty()) {
        qCWarning(KCM_FCITX5) << "Cannot save config - controller:" << dbus_->controller()
                  << "uri:" << uri_;
        return;
    }
    QDBusVariant var(value());
    dbus_->controller()->SetConfig(uri_, var);
    qCDebug(KCM_FCITX5) << "Exiting save";
}

void ConfigWidget::setValue(const QVariant &value) {
    qCDebug(KCM_FCITX5) << "Setting config values";
    if (!initialized_) {
        qCWarning(KCM_FCITX5) << "Cannot set value - widget not initialized";
        return;
    }

    dontEmitChanged_ = true;
    auto optionWidgets = findChildren<OptionWidget *>();
    QVariantMap map;
    if (value.canConvert<QDBusArgument>()) {
        auto argument = qvariant_cast<QDBusArgument>(value);
        argument >> map;
    } else {
        map = value.toMap();
    }
    for (auto optionWidget : optionWidgets) {
        optionWidget->readValueFrom(map);
    }
    dontEmitChanged_ = false;
    qCDebug(KCM_FCITX5) << "Exiting setValue";
}

QVariant ConfigWidget::value() const {
    // qCDebug(KCM_FCITX5) << "Entering value";
    QVariantMap map;
    auto optionWidgets = findChildren<OptionWidget *>();
    for (auto optionWidget : optionWidgets) {
        optionWidget->writeValueTo(map);
    }
    // qCDebug(KCM_FCITX5) << "Exiting value";
    return map;
}

void ConfigWidget::buttonClicked(QDialogButtonBox::StandardButton button) {
    qCDebug(KCM_FCITX5) << "Entering buttonClicked with button:" << button;
    if (button == QDialogButtonBox::RestoreDefaults) {
        qCInfo(KCM_FCITX5) << "RestoreDefaults button clicked.";
        auto optionWidgets = findChildren<OptionWidget *>();
        for (auto optionWidget : optionWidgets) {
            optionWidget->restoreToDefault();
        }
    } else if (button == QDialogButtonBox::Ok) {
        qCInfo(KCM_FCITX5) << "Ok button clicked, saving.";
        save();
    }
    qCDebug(KCM_FCITX5) << "Exiting buttonClicked";
}

void ConfigWidget::setupWidget(QWidget *widget, const QString &type,
                               const QString &path) {
    qCDebug(KCM_FCITX5) << "Setting up widget";
    if (!desc_.contains(type)) {
        qCCritical(KCM_FCITX5) << type << " type does not exists.";
    }

    auto layout = new QFormLayout(widget);
    const auto &options = desc_[type];
    for (auto &option : options) {
        addOptionWidget(layout, option, joinPath(path, option.name()));
    }

    widget->setLayout(layout);
    qCDebug(KCM_FCITX5) << "Exiting setupWidget";
}

void ConfigWidget::addOptionWidget(QFormLayout *layout,
                                   const FcitxQtConfigOption &option,
                                   const QString &path) {
    qCDebug(KCM_FCITX5) << "Entering addOptionWidget for path:" << path;
    if (auto optionWidget =
            OptionWidget::addWidget(layout, option, path, this)) {
        connect(optionWidget, &OptionWidget::valueChanged, this,
                &ConfigWidget::doChanged);
    } else if (desc_.contains(option.type())) {
        qCDebug(KCM_FCITX5) << "Adding group box for option:" << option.description();
        QGroupBox *box = new QGroupBox;
        box->setTitle(option.description());
        QVBoxLayout *innerLayout = new QVBoxLayout;
        QWidget *widget = new QWidget;
        setupWidget(widget, option.type(), path);
        innerLayout->addWidget(widget);
        box->setLayout(innerLayout);
        layout->addRow(box);
    } else {
        qCDebug(KCM_FCITX5) << "Unknown option type:" << option.type()
                           << "for path:" << path;
    }
    qCDebug(KCM_FCITX5) << "Exiting addOptionWidget";
}

QDialog *ConfigWidget::configDialog(QWidget *parent, DBusProvider *dbus,
                                    const QString &uri, const QString &title) {
    qCDebug(KCM_FCITX5) << "Entering configDialog for URI:" << uri;
    auto configPage = new ConfigWidget(uri, dbus);
    configPage->requestConfig(true);
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
                             QDialogButtonBox::RestoreDefaults);

    buttonBox->button(QDialogButtonBox::Ok)->setText(_("&OK"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(_("&Cancel"));
    buttonBox->button(QDialogButtonBox::RestoreDefaults)
        ->setText(_("Restore &Defaults"));

    auto configPageWrapper = new VerticalScrollArea;
    configPageWrapper->setWidget(configPage);
    dialogLayout->addWidget(configPageWrapper);
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

    qCDebug(KCM_FCITX5) << "Exiting configDialog";
    return dialog;
}

void ConfigWidget::doChanged() {
    qCDebug(KCM_FCITX5) << "Entering doChanged";
    if (dontEmitChanged_) {
        qCDebug(KCM_FCITX5) << "Change emission is disabled, exiting.";
        return;
    }
    Q_EMIT changed();
    qCDebug(KCM_FCITX5) << "Exiting doChanged";
}

ConfigWidget *getConfigWidget(QWidget *widget) {
    qCDebug(KCM_FCITX5) << "Entering getConfigWidget";
    widget = widget->parentWidget();
    ConfigWidget *configWidget;
    while (widget) {
        configWidget = qobject_cast<ConfigWidget *>(widget);
        if (configWidget) {
            break;
        }
        widget = widget->parentWidget();
    }
    qCDebug(KCM_FCITX5) << "Exiting getConfigWidget, found config widget";
    return configWidget;
}

} // namespace kcm
} // namespace fcitx
