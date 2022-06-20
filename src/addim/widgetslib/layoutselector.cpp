/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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
#include <QX11Info>
#include <fcitx-utils/i18n.h>
#include <fcitxqtcontrollerproxy.h>
#include <fcitxqtdbustypes.h>

namespace fcitx {
namespace kcm {

LayoutSelector::LayoutSelector(DBusProvider *dbus, QWidget *parent)
    : QWidget(parent)
    , ui_(std::make_unique<Ui::LayoutSelector>())
    , dbus_(dbus)
    , layoutProvider_(new LayoutProvider(dbus, this))
{
    if (QX11Info::isPlatformX11()) {
        keyboardLayoutWidget_ = new KeyboardLayoutWidget(this);
        keyboardLayoutWidget_->setMinimumSize(QSize(360, 130));
        keyboardLayoutWidget_->setContentsMargins(0,-20,0,0);
        keyboardLayoutWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        //keyboardLayoutWidget_->resize(this->size());
    }
}

LayoutSelector::~LayoutSelector() {}

LayoutSelector *LayoutSelector::selectLayout(QWidget *parent, DBusProvider *dbus,
                             const QString &title, const QString &layout)
{
    auto mainLayout = new QVBoxLayout(parent);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    auto layoutSelector = new LayoutSelector(dbus, parent);
    mainLayout->addWidget(layoutSelector);
    layoutSelector->setLayout(layout, "");

    return layoutSelector;
}

void LayoutSelector::setLayout(const QString &layout, const QString &variant)
{
    if (!layoutProvider_->loaded()) {
        preSelectLayout_ = layout;
        preSelectVariant_ = variant;
        return;
    }

    preSelectLayout_.clear();
    preSelectVariant_.clear();
}

QPair<QString, QString> LayoutSelector::layout() const
{
    return QPair<QString, QString> ();
}

void LayoutSelector::languageComboBoxChanged()
{
}

void LayoutSelector::layoutComboBoxChanged()
{

}

void LayoutSelector::variantComboBoxChanged()
{
}

} // namespace kcm
} // namespace fcitx
