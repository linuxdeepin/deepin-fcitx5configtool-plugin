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

#include "imactivityitem.h"
#include "publisher/publisherdef.h"
#include <DToolButton>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <QTimer>
#include <QPainterPath>
#include "window/settingsdef.h"
using namespace Dtk::Widget;
namespace dcc_fcitx_configtool {
namespace widgets {

FcitxIMActivityItem::FcitxIMActivityItem(FcitxQtInputMethodItem *item, itemPosition index, QWidget *parent)
    : FcitxSettingsItem(parent)
    , m_item(item)
    , m_index(index)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 10, 0);
    m_labelText = new FcitxShortenLabel("", this);
    DFontSizeManager::instance()->bind(m_labelText, DFontSizeManager::T6);
    m_labelText->setShortenText("    " + item->name());
    m_labelText->setAccessibleName(item->name());
    m_labelText->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_layout->addWidget(m_labelText);
    m_upBtn = new DToolButton(this);
    m_upBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_downBtn = new DToolButton(this);
    m_downBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_configBtn = new DToolButton(this);
    m_configBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_deleteLabel = new ClickLabel (this);
    m_upBtn->setIcon(QIcon::fromTheme("arrow_up"));
    m_upBtn->setAccessibleName(item->name()+":arrow_up");
    m_downBtn->setIcon(QIcon::fromTheme("arrow_down"));
    m_downBtn->setAccessibleName(item->name()+":arrow_down");
    m_configBtn->setIcon(QIcon::fromTheme("setting"));
    m_configBtn->setAccessibleName(item->name()+":setting");
    m_deleteLabel->setIcon(DStyle::standardIcon(QApplication::style(), DStyle::SP_DeleteButton));
    m_deleteLabel->setAccessibleName(item->name()+":delete");
    m_deleteLabel->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);


    m_layout->addWidget(m_downBtn);
    m_layout->addWidget(m_upBtn);
    m_layout->addWidget(m_configBtn);
    m_layout->addWidget(m_deleteLabel, 0, Qt::AlignRight);

    m_deleteLabel->hide();
    m_upBtn->hide();
    m_configBtn->hide();
    m_downBtn->hide();

    connect(m_upBtn, &DToolButton::clicked, this, &FcitxIMActivityItem::onUpItem);
    connect(m_downBtn, &DToolButton::clicked, this, &FcitxIMActivityItem::onDownItem);
    connect(m_configBtn, &DToolButton::clicked, this, &FcitxIMActivityItem::onConfigItem);
    connect(m_deleteLabel, &ClickLabel::clicked, this, &FcitxIMActivityItem::onDeleteItem);

    this->setFixedHeight(40);
    this->setLayout(m_layout);
}

FcitxIMActivityItem::~FcitxIMActivityItem()
{
}

void FcitxIMActivityItem::editSwitch(const bool &flag)
{
    m_isEdit = flag;
    if (m_isEdit) {
        m_deleteLabel->show();
        m_configBtn->hide();
        m_upBtn->hide();
        m_downBtn->hide();
    } else {
        m_deleteLabel->hide();
    }
}

void FcitxIMActivityItem::paintEvent(QPaintEvent *event)
{
    QPainter painter( this);
    painter.fillRect(this->rect(), DGuiApplicationHelper::instance()->applicationPalette().window());
    const int radius = 8;
    QRect paintRect = this->rect();
    QPainterPath path;
    if(m_index == firstItem) {
        path.moveTo(paintRect.bottomRight());
        path.lineTo(paintRect.topRight() + QPoint(0, radius));
        path.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)),
                         QSize(radius * 2, radius * 2)), 0, 90);
        path.lineTo(paintRect.topLeft() + QPoint(radius, 0));
        path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
        path.lineTo(paintRect.bottomLeft());
        path.lineTo(paintRect.bottomRight());
    } if(m_index == lastItem) {
        path.moveTo(paintRect.bottomRight() - QPoint(0, radius));
        path.lineTo(paintRect.topRight());
        path.lineTo(paintRect.topLeft());
        path.lineTo(paintRect.bottomLeft() - QPoint(0, radius));
        path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)),
                         QSize(radius * 2, radius * 2)), 180, 90);
        path.lineTo(paintRect.bottomLeft() + QPoint(radius, 0));
        path.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)),
                         QSize(radius * 2, radius * 2)), 270, 90);
    } if(m_index == otherItem) {
        path.moveTo(paintRect.bottomRight());
        path.lineTo(paintRect.topRight());
        path.lineTo(paintRect.topLeft());
        path.lineTo(paintRect.bottomLeft());
        path.lineTo(paintRect.bottomRight());
    } if(m_index == onlyoneItem) {
        path.moveTo(paintRect.bottomRight() - QPoint(0, radius));
        path.lineTo(paintRect.topRight() + QPoint(0, radius));
        path.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)),
                         QSize(radius * 2, radius * 2)), 0, 90);
        path.lineTo(paintRect.topLeft() + QPoint(radius, 0));
        path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
        path.lineTo(paintRect.bottomLeft() - QPoint(0, radius));
        path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)),
                         QSize(radius * 2, radius * 2)), 180, 90);
        path.lineTo(paintRect.bottomLeft() + QPoint(radius, 0));
        path.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)),
                         QSize(radius * 2, radius * 2)), 270, 90);
    }
    if(isSelected()) {
        QColor color = DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
        if(isDraged()) {
            color.setAlpha(160);
        }
        painter.fillPath(path, color);
    } else if(isEntered() && (!isSelected())) {
        QColor color = DGuiApplicationHelper::instance()->applicationPalette().light().color();
        if(isDraged()) {
            color.setAlpha(160);
        }
        painter.fillPath(path, color);
    } else {
        DPalette p;
        QColor color = Qt::red;
        if(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
            color = QColor("#323232");
            color.setAlpha(230);
        } else {
            color = DGuiApplicationHelper::instance()->applicationPalette().frameBorder().color();
        }
        if(isDraged()) {
            color = DGuiApplicationHelper::instance()->applicationPalette().light().color();
            color.setAlpha(160);
        }
        painter.fillPath(path, color);
    }
    return FcitxSettingsItem::paintEvent(event);
}

void FcitxIMActivityItem::mouseMoveEvent(QMouseEvent *e)
{
    update();
    return FcitxSettingsItem::mouseMoveEvent(e);
}

void FcitxIMActivityItem::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit selectItem(this, true);
   // setSelectStatus(true);
    this->setFocus();
    return FcitxSettingsItem::mousePressEvent(event);
}

void FcitxIMActivityItem::mouseReleaseEvent(QMouseEvent *event)
{
    update();
    return FcitxSettingsItem::mouseReleaseEvent(event);
}

void FcitxIMActivityItem::focusOutEvent(QFocusEvent *e)
{
    emit itemSelect(false);
    setSelectStatus(false, 0, 0);
    setIsSelected(false);
    qDebug() << "focusOutEvent";
    update();
}

void FcitxIMActivityItem::setSelectStatus(const bool &isSelect, int index, int count)
{
    if(isSelected() == isSelect) {
        return;
    }

    qDebug() << "m_isSelected = " << isSelect << "index " << index;

    if (isSelect)
        setIsSelected(true);
    else {
        setIsSelected(false);
    }
    QPalette pe = m_labelText->palette();
    if (!m_isEdit && isSelect) {
        if (count <= 1) {
            m_upBtn->setEnabled(false);
            m_downBtn->setEnabled(false);
        } else if (index == 0) {
            m_upBtn->setEnabled(false);
            m_downBtn->setEnabled(true);
        } else if (index == count - 1) {
            m_upBtn->setEnabled(true);
            m_downBtn->setEnabled(false);
        }
        pe.setColor(QPalette::WindowText, Qt::white);
        m_labelText->setPalette(pe);
        m_configBtn->show();
        m_upBtn->show();
        m_downBtn->show();
        //update();
    } else {
        pe.setColor(QPalette::WindowText, QColor("#323232"));
        m_labelText->setPalette(pe);
        m_configBtn->hide();
        m_upBtn->hide();
        m_downBtn->hide();
    }
    //this->update(rect());
    update();
}

void FcitxIMActivityItem::setEnterStatus(const bool &isEnter, int index, int count)
{
    //qDebug() << "setEnterStatus " << this;
    if (!m_isEdit && isEnter) {
        if (count <= 1) {
            m_upBtn->setEnabled(false);
            m_downBtn->setEnabled(false);
        } else if (index == 0) {
            m_upBtn->setEnabled(false);
            m_downBtn->setEnabled(true);
        } else if (index == count - 1) {
            m_upBtn->setEnabled(true);
            m_downBtn->setEnabled(false);
        }
        m_configBtn->show();
        m_upBtn->show();
        m_downBtn->show();
        //update();
    } else {
        m_configBtn->hide();
        m_upBtn->hide();
        m_downBtn->hide();
    }
    update();
}

void FcitxIMActivityItem::onUpItem()
{
    emit upBtnClicked(m_item);
    //update();
}

void FcitxIMActivityItem::onDownItem()
{
    emit downBtnClicked(m_item);
    //update();
}

void FcitxIMActivityItem::onConfigItem()
{
    emit configBtnClicked(m_item);
}

void FcitxIMActivityItem::onDeleteItem()
{
    emit deleteBtnClicked(m_item);
}

void FcitxIMActivityItem::enterEvent(QEvent *event)
{
    //qDebug() << "enterEvent " << event->type();
    emit enterItem(this, true);
    setIsEntered(true);
    FcitxSettingsItem::enterEvent(event);
}

void FcitxIMActivityItem::leaveEvent(QEvent *event)
{
    //qDebug() << "leaveEvent";
    emit enterItem(this, false);
    setIsEntered(false);
    FcitxSettingsItem::leaveEvent(event);
}

void ToolButton::paintEvent(QPaintEvent *e)
{
    if (isEnabled()) {
        QToolButton::paintEvent(e);
    } else {
        QPainter p(this);
        p.drawPixmap({(width() - 16) / 2, (height() - 16) / 2, 16, 16}, icon().pixmap(16, 16, QIcon::Mode::Disabled));
    }
}



} // namespace widgets
} // namespace dcc_fcitx_configtool
