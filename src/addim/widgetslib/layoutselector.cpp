// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#include "layoutselector.h"

#include "dbusprovider.h"
#include "keyboardlayoutwidget.h"
#include "layoutmodel.h"
#include "ui_layoutselector.h"

#include <fcitx-utils/i18n.h>
#include <fcitxqtcontrollerproxy.h>
#include <fcitxqtdbustypes.h>

#include <QDBusPendingCallWatcher>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QStringListModel>
#include <QX11Info>

namespace fcitx {
namespace addim {

LayoutSelector::LayoutSelector(DBusProvider *dbus, QWidget *parent)
    : QWidget(parent)
    , m_ui(std::make_unique<Ui::LayoutSelector>())
    , m_dbus(dbus)
    , m_layoutProvider(new LayoutProvider(dbus, this))
{
    m_ui->setupUi(this);
    m_ui->layoutComboBox->setVisible(false);
    m_ui->languageComboBox->setVisible(false);
    m_ui->variantComboBox->setVisible(false);

    m_ui->languageComboBox->setModel(m_layoutProvider->languageModel());
    m_ui->layoutComboBox->setModel(m_layoutProvider->layoutModel());
    m_ui->variantComboBox->setModel(m_layoutProvider->variantModel());

    connect(m_ui->languageComboBox,
            qOverload<int>(&QComboBox::currentIndexChanged), this,
            &LayoutSelector::languageComboBoxChanged);
    connect(m_ui->layoutComboBox,
            qOverload<int>(&QComboBox::currentIndexChanged), this,
            &LayoutSelector::layoutComboBoxChanged);
    connect(m_ui->variantComboBox,
            qOverload<int>(&QComboBox::currentIndexChanged), this,
            &LayoutSelector::variantComboBoxChanged);

    if (QX11Info::isPlatformX11()) {
        m_keyboardLayoutWidget = new KeyboardLayoutWidget(this);
        m_keyboardLayoutWidget->setMinimumSize(QSize(360, 130));
        m_keyboardLayoutWidget->setContentsMargins(0,-20,0,0);
        m_keyboardLayoutWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
    int index = m_layoutProvider->layoutIndex(layout);
    qInfo("set keyboard layout: layoutName=%s, index=%d", layout.toStdString().c_str(), index);
    m_ui->layoutComboBox->setCurrentIndex(index);
    if (!m_layoutProvider->loaded()) {
        m_preSelectLayout = layout;
        m_preSelectVariant = variant;
        return;
    }

    m_preSelectLayout.clear();
    m_preSelectVariant.clear();
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
    if (m_ui->layoutComboBox->currentIndex() < 0) {
        return;
    }

    m_layoutProvider->setVariantInfo(
        m_ui->layoutComboBox->currentData(LayoutInfoRole)
            .value<FcitxQtLayoutInfo>());
}

void LayoutSelector::variantComboBoxChanged()
{
    if (!m_keyboardLayoutWidget) {
        return;
    }

    auto layout = m_ui->layoutComboBox->currentData().toString();
    auto variant = m_ui->variantComboBox->currentData().toString();
    if (layout.isEmpty()) {
        m_keyboardLayoutWidget->setVisible(false);
    } else {
        m_keyboardLayoutWidget->setKeyboardLayout(layout, variant);
        m_keyboardLayoutWidget->setVisible(true);
    }
}

} // namespace addim
} // namespace fcitx
