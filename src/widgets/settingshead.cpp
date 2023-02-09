// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingshead.h"
#include "labels/normallabel.h"

#include <DCommandLinkButton>

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QApplication>
#include <DFontSizeManager>
#include "configsetting/configsetting.h"

using namespace dcc_fcitx_configtool::widgets;
DWIDGET_USE_NAMESPACE

FcitxSettingsHead::FcitxSettingsHead(bool isEdit, QFrame *parent)
    : FcitxSettingsItem(parent)
    , m_title(new FcitxTitleLabel)
    , m_deleteBtn(new DIconButton(DStyle::SP_DecreaseElement))
    , m_addBtn(new DIconButton(DStyle::SP_IncreaseElement))
    , m_state(Cancel)
    , m_setting(new ConfigSettings())
{
    m_title->setObjectName("SettingsHeadTitle");
    //m_deleteBtn->setFocusPolicy(Qt::NoFocus);
    DFontSizeManager::instance()->bind(m_title, DFontSizeManager::T5, QFont::DemiBold);
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 10, 0);
    mainLayout->addWidget(m_title);
    mainLayout->addStretch();
    m_deleteBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_addBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    if(isEdit) {
        m_deleteBtn->setEnabled(false);
        //m_deleteBtn->setFixedSize(36,36);
        //m_addBtn->setFixedSize(36,36);
        mainLayout->addWidget(m_deleteBtn);
        mainLayout->addSpacing(10);
        mainLayout->addWidget(m_addBtn);
    }
    hideDeleteButton();
    setAddButtonEnable();
    hideAddButton();
    setLayout(mainLayout);

    connect(m_deleteBtn, &DCommandLinkButton::pressed, this, [=](){
        qDebug() << "DCommandLinkButton clicked";
        emit deleteBtnClicked();
        m_deleteBtn->setEnabled(false);
    });
    connect(m_addBtn, &DCommandLinkButton::clicked, this, &FcitxSettingsHead::addBtnClicked);
}

FcitxSettingsHead::~FcitxSettingsHead()
{
}

void FcitxSettingsHead::setTitle(const QString &title)
{
    m_title->setText(title);
    m_deleteBtn->setAccessibleName(title);
}

void FcitxSettingsHead::setDeleteButtonEnable(bool state)
{
    auto del = m_setting->GetKeyValue(DCONFIG_DELETE);
    bool enable = (del == WindowState::Disable);
    if (enable) {
        state = false;
    }
    m_deleteBtn->setEnabled(state);
}

void FcitxSettingsHead::hideDeleteButton()
{
    auto del = m_setting->GetKeyValue(DCONFIG_DELETE);

    bool enable = (del == WindowState::Hide);

    if (enable) {
        m_deleteBtn->hide();
    }
}

void FcitxSettingsHead::setAddButtonEnable()
{
    auto del = m_setting->GetKeyValue(DCONFIG_ADD_IM);
    bool enable = (del == WindowState::Disable);
    m_addBtn->setEnabled(!enable);
}

void FcitxSettingsHead::hideAddButton()
{
    auto del = m_setting->GetKeyValue(DCONFIG_ADD_IM);
    bool enable = (del == WindowState::Hide);
    if (enable) {
        m_addBtn->hide();
    }
}

FcitxTitleLabel* FcitxSettingsHead::getTitleLabel()
{
    return m_title;
}
