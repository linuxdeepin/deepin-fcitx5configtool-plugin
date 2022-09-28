/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "settingshead.h"
#include "labels/normallabel.h"

#include <DCommandLinkButton>

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QApplication>
#include <DFontSizeManager>
#include "window/settingsdef.h"

using namespace dcc_fcitx_configtool::widgets;
DWIDGET_USE_NAMESPACE

FcitxSettingsHead::FcitxSettingsHead(bool isEdit, QFrame *parent)
    : FcitxSettingsItem(parent)
    , m_title(new FcitxTitleLabel)
    , m_deleteBtn(new DIconButton(DStyle::SP_DecreaseElement))
    , m_addBtn(new DIconButton(DStyle::SP_IncreaseElement))
    , m_state(Cancel)
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

void FcitxSettingsHead::setDeleteEnable(bool state)
{
    m_deleteBtn->setEnabled(state);
}

FcitxTitleLabel* FcitxSettingsHead::getTitleLabel()
{
    return m_title;
}
