// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGSHEAD_H
#define SETTINGSHEAD_H

#include "settingsitem.h"
#include "titlelabel.h"

#include <QPushButton>
#include <DIconButton>

DWIDGET_BEGIN_NAMESPACE
class DCommandLinkButton;
DWIDGET_END_NAMESPACE
class ConfigSettings;
namespace dcc_fcitx_configtool {
namespace widgets {

class FcitxNormalLabel;

class FcitxSettingsHead : public FcitxSettingsItem
{
    Q_OBJECT

public:
    enum State {
        Edit,
        Cancel
    };

public:
    explicit FcitxSettingsHead(bool isEdit = false, QFrame *parent = nullptr);
    ~FcitxSettingsHead();

    void setTitle(const QString &title);
    void setDeleteButtonEnable(bool state = true);
    void hideDeleteButton();
    void hideAddButton();
    void setAddButtonEnable();
    FcitxTitleLabel* getTitleLabel();

Q_SIGNALS:
    void editChanged(bool edit);
    void deleteBtnClicked();
    void addBtnClicked();
private:
    FcitxTitleLabel *m_title;
    DTK_WIDGET_NAMESPACE::DIconButton *m_deleteBtn;
    DTK_WIDGET_NAMESPACE::DIconButton *m_addBtn;
    bool m_editVisible;
    State m_state;
    ConfigSettings *m_setting;
};

} // namespace widgets
} // namespace dcc_fcitx_configtool

#endif // SETTINGSHEAD_H
