// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMACTIVITYITEM_H
#define IMACTIVITYITEM_H
#include "settingsitem.h"
#include "labels/shortenlabel.h"
#include "labels/clicklabel.h"
#include "publisher/publisherdef.h"
#include <DToolButton>

using namespace dcc_fcitx_configtool::widgets;
using namespace Dtk::Widget;

class ConfigSettings;
namespace dcc_fcitx_configtool {
namespace widgets {

class ToolButton : public DToolButton
{
    Q_OBJECT
public:
    using DToolButton::DToolButton;

protected:
    void paintEvent(QPaintEvent *e) override;
};

class FcitxIMActivityItem : public FcitxSettingsItem
{
    Q_OBJECT
public:
    FcitxIMActivityItem(FcitxQtInputMethodItem *item, itemPosition index, QWidget *parent = nullptr);
    ~FcitxIMActivityItem() override;
    void editSwitch(const bool &flag);
    void setSelectStatus(const bool &isSelect, int index, int count);
    void setEnterStatus(const bool &isEnter, int index, int count);
    void setIndex(itemPosition i) {m_index = i;}
signals:
    void upBtnClicked(FcitxQtInputMethodItem*);
    void downBtnClicked(FcitxQtInputMethodItem*);
    void configBtnClicked(FcitxQtInputMethodItem*);
    void deleteBtnClicked(FcitxQtInputMethodItem*);
    void selectItem(FcitxSettingsItem * item, bool selected);
    void enterItem(FcitxSettingsItem * item, bool entered);
    void itemSelect(bool selected);

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void focusOutEvent(QFocusEvent *e) override;
    void enableSortButton();
    void enableSettingButton();

private slots:
    void onUpItem();
    void onDownItem();
    void onConfigItem();
    void onDeleteItem();
public:
    FcitxQtInputMethodItem *m_item;
    FcitxShortenLabel *m_labelText {nullptr};
    QHBoxLayout *m_layout {nullptr};
    DToolButton *m_upBtn {nullptr};
    DToolButton *m_downBtn {nullptr};
    DToolButton *m_configBtn {nullptr};
    ClickLabel *m_deleteLabel {nullptr};
    bool m_isEdit {false};
private:
    bool m_isEntered {false};
    itemPosition m_index; //0:第一个， -1：最后一个， -2：只有一个
    bool m_isPressed {false};
    ConfigSettings *m_setting;
};

} // namespace widgets
} // namespace dcc_fcitx_configtool

#endif // IMACTIVITYITEM_H
