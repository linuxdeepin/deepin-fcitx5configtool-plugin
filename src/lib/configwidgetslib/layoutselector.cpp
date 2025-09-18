/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <logging.h>
#include "layoutselector.h"
#include "dbusprovider.h"
#include "keyboardlayoutwidget.h"
#include "layoutmodel.h"
#include "ui_layoutselector.h"
#include <QDBusPendingCallWatcher>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QStringListModel>
#include <fcitx-utils/i18n.h>
#include <fcitxqtcontrollerproxy.h>
#include <fcitxqtdbustypes.h>

namespace fcitx {
namespace kcm {

LayoutSelector::LayoutSelector(DBusProvider *dbus, QWidget *parent)
    : QWidget(parent), ui_(std::make_unique<Ui::LayoutSelector>()), dbus_(dbus),
      layoutProvider_(new LayoutProvider(dbus, this)) {
    qCDebug(KCM_FCITX5) << "LayoutSelector created with dbus";
    ui_->setupUi(this);

    ui_->languageComboBox->setModel(layoutProvider_->languageModel());
    ui_->layoutComboBox->setModel(layoutProvider_->layoutModel());
    ui_->variantComboBox->setModel(layoutProvider_->variantModel());

    connect(layoutProvider_, &LayoutProvider::loadedChanged, this, [this]() {
        if (layoutProvider_->loaded()) {
            setLayout(preSelectLayout_, preSelectVariant_);
        }
    });

    connect(ui_->languageComboBox,
            qOverload<int>(&QComboBox::currentIndexChanged), this,
            &LayoutSelector::languageComboBoxChanged);
    connect(ui_->layoutComboBox,
            qOverload<int>(&QComboBox::currentIndexChanged), this,
            &LayoutSelector::layoutComboBoxChanged);
    connect(ui_->variantComboBox,
            qOverload<int>(&QComboBox::currentIndexChanged), this,
            &LayoutSelector::variantComboBoxChanged);
    if (qApp->platformName() == "xcb") {
        keyboardLayoutWidget_ = new KeyboardLayoutWidget(this);
        keyboardLayoutWidget_->setMinimumSize(QSize(400, 200));
        keyboardLayoutWidget_->setSizePolicy(QSizePolicy::Expanding,
                                             QSizePolicy::Expanding);
        ui_->verticalLayout->addWidget(keyboardLayoutWidget_);
    }
    qCDebug(KCM_FCITX5) << "Exiting LayoutSelector constructor";
}

LayoutSelector::~LayoutSelector() {
    // qCDebug(KCM_FCITX5) << "LayoutSelector destroyed";
}

QPair<QString, QString>
LayoutSelector::selectLayout(QWidget *parent, DBusProvider *dbus,
                             const QString &title, const QString &layout,
                             const QString &variant, bool *ok) {
    qCDebug(KCM_FCITX5) << "Selecting layout dialog";
    QPointer<QDialog> dialog(new QDialog(parent));
    auto mainLayout = new QVBoxLayout(dialog);
    dialog->setLayout(mainLayout);
    dialog->setWindowTitle(title);
    auto layoutSelector = new LayoutSelector(dbus, dialog);
    mainLayout->addWidget(layoutSelector);
    layoutSelector->setLayout(layout, variant);

    auto buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, dialog);
    buttonBox->button(QDialogButtonBox::Ok)->setText(_("&OK"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(_("&Clear"));
    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    auto ret = dialog->exec();
    if (ok) {
        qCDebug(KCM_FCITX5) << "Setting ok to:" << ret;
        *ok = !!ret;
    }
    if (ret) {
        qCDebug(KCM_FCITX5) << "Returning layout:" << layoutSelector->layout();
        return layoutSelector->layout();
    } else {
        qCDebug(KCM_FCITX5) << "Returning empty layout";
        return {};
    }
}

void LayoutSelector::setLayout(const QString &layout, const QString &variant) {
    qCDebug(KCM_FCITX5) << "Entering setLayout with layout:" << layout << "variant:" << variant;
    if (!layoutProvider_->loaded()) {
        qCDebug(KCM_FCITX5) << "Layout provider not loaded yet, caching selection:"
                           << "layout:" << preSelectLayout_
                           << "variant:" << preSelectVariant_;
        preSelectLayout_ = layout;
        preSelectVariant_ = variant;
        return;
    }
    ui_->languageComboBox->setCurrentIndex(0);
    ui_->layoutComboBox->setCurrentIndex(layoutProvider_->layoutIndex(layout));
    if (variant.isEmpty()) {
        ui_->variantComboBox->setCurrentIndex(0);
    } else {
        ui_->variantComboBox->setCurrentIndex(
            layoutProvider_->variantIndex(variant));
    }
    preSelectLayout_.clear();
    preSelectVariant_.clear();
    qCDebug(KCM_FCITX5) << "Exiting setLayout";
}

QPair<QString, QString> LayoutSelector::layout() const {
    // qCDebug(KCM_FCITX5) << "Entering layout";
    return {ui_->layoutComboBox->currentData().toString(),
            ui_->variantComboBox->currentData().toString()};
}

void LayoutSelector::languageComboBoxChanged() {
    // qCDebug(KCM_FCITX5) << "Entering languageComboBoxChanged";
    layoutProvider_->layoutModel()->setLanguage(
        ui_->languageComboBox->currentData().toString());
}

void LayoutSelector::layoutComboBoxChanged() {
    // qCDebug(KCM_FCITX5) << "Entering layoutComboBoxChanged";
    ui_->variantComboBox->clear();
    if (ui_->layoutComboBox->currentIndex() < 0) {
        qCWarning(KCM_FCITX5) << "Invalid layout index:"
                             << ui_->layoutComboBox->currentIndex();
        return;
    }

    layoutProvider_->setVariantInfo(
        ui_->layoutComboBox->currentData(LayoutInfoRole)
            .value<FcitxQtLayoutInfo>());
    ui_->variantComboBox->setCurrentIndex(0);
    // qCDebug(KCM_FCITX5) << "Exiting layoutComboBoxChanged";
}

void LayoutSelector::variantComboBoxChanged() {
    qCDebug(KCM_FCITX5) << "Entering variantComboBoxChanged";
    if (!keyboardLayoutWidget_) {
        qCDebug(KCM_FCITX5) << "Exiting variantComboBoxChanged (no keyboard widget)";
        return;
    }

    auto layout = ui_->layoutComboBox->currentData().toString();
    auto variant = ui_->variantComboBox->currentData().toString();
    if (layout.isEmpty()) {
        qCDebug(KCM_FCITX5) << "No layout selected, hiding keyboard widget";
        keyboardLayoutWidget_->setVisible(false);
    } else {
        qCDebug(KCM_FCITX5) << "Showing keyboard widget for layout";
        keyboardLayoutWidget_->setKeyboardLayout(layout, variant);
        keyboardLayoutWidget_->setVisible(true);
    }
    qCDebug(KCM_FCITX5) << "Exiting variantComboBoxChanged";
}

} // namespace kcm
} // namespace fcitx
