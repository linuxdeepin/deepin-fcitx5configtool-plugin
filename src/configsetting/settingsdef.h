// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

enum class WindowState {
    UnKnow = -1,
    Enable = 0,
    Disable,
    Hide
};

#define DCONFIG_DELETE "deleteButton"  //输入法管理-删除
#define DCONFIG_ADJUST_BUTTON "adjustButton"  //输入法管理-调整顺序
#define DCONFIG_SETTING_BUTTON "settingButton" //输入法管理-设置
#define DCONFIG_ADD_IM "addIMButton" //输入法管理-添加输入法

#define DCONFIG_SHORTCUT_RESTORE "shortcutRestore"  //快捷键-恢复默认
#define DCONFIG_SHORTCUT_SWITCHTORFIRST "shortcutSwitchOrFirstImContent"  //快捷键-切换至首位输入法界面
#define DCONFIG_SHORTCUT_SWITCHTORFIRSTFUN "shortcutSwitchOrFirstImContentFun"  //快捷键-切换至首位输入法界面功能

#define DCONFIG_ADVANCE_SETTING "advancedSetting" //高级设置按钮


