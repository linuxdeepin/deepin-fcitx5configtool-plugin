/*
* Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
*
* Author:     zhaoyue <zhaoyue@uniontech.com>
*
* Maintainer: zhaoyue <zhaoyue@uniontech.com>
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


