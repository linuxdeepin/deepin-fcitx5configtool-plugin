/*
* Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
*
* Author:     liuwenhao <liuwenhao@uniontech.com>
*
* Maintainer: liuwenhao <liuwenhao@uniontech.com>
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
#include "imsettingsitem.h"
#include "settingsitem.h"
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QLabel>
#include <QApplication>
#include <QFrame>
#include <DPalette>
#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE
namespace dcc_fcitx_configtool {
namespace widgets {
FcitxIMSettingsItem::FcitxIMSettingsItem(QString str, QFrame *parent)
    : FcitxSettingsItem(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(10, 0, 10, 0);
    m_labelText = new FcitxShortenLabel(str, this);
    DFontSizeManager::instance()->bind(m_labelText, DFontSizeManager::T6);
    m_labelIcon = new QLabel(this);
    QIcon icon = DStyle::standardIcon(QApplication::style(), DStyle::SP_IndicatorChecked);
    m_labelIcon->setPixmap(icon.pixmap(QSize(20, 20)));
    m_labelIcon->setFixedWidth(20);
    m_labelIcon->hide();
    m_layout->addWidget(m_labelText);
    m_layout->addWidget(m_labelIcon);
    this->setFixedHeight(40);
    this->setLayout(m_layout);
}

FcitxIMSettingsItem::~FcitxIMSettingsItem()
{
}

void FcitxIMSettingsItem::setFcitxItem(const FcitxQtInputMethodItem &item)
{
    m_item = item;
    m_labelText->setShortenText(m_item.name());
    setAccessibleName(m_item.name());
}

void FcitxIMSettingsItem::setFilterStr(QString str)
{
    if (!(m_item.name().indexOf(str, Qt::CaseInsensitive) != -1
          || m_item.uniqueName().indexOf(str, Qt::CaseInsensitive) != -1
          || m_item.langCode().indexOf(str, Qt::CaseInsensitive) != -1)) {
        this->hide();
        this->setVisible(false);
    } else {
        this->show();
        this->setVisible(true);
    }
}

void FcitxIMSettingsItem::setItemSelected(bool status)
{
    if (status) {
        m_labelIcon->show();
    } else {
        m_labelIcon->hide();
    }
}

void FcitxIMSettingsItem::mousePressEvent(QMouseEvent *event)
{
    setItemSelected(true);
    emit itemClicked(this);

    FcitxSettingsItem::mousePressEvent(event);
}

void FcitxIMSettingsItem::enterEvent(QEvent *event)
{
    if (m_bgGroup)
        m_bgGroup->setBackgroundRole(DPalette::ObviousBackground);

    FcitxSettingsItem::enterEvent(event);
}

void FcitxIMSettingsItem::leaveEvent(QEvent *event)
{
    if (m_bgGroup)
        m_bgGroup->setBackgroundRole(DPalette::ItemBackground);

    FcitxSettingsItem::leaveEvent(event);
}

} // namespace widgets
} // namespace dcc_fcitx_configtool
