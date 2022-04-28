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
#ifndef IMSETTINGWINDOW_H
#define IMSETTINGWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

#include "publisher/publisherdef.h"

namespace dcc_fcitx_configtool {
namespace widgets {
class FcitxSettingsGroup;
class FcitxSettingsHead;
class FcitxComBoboxSettingsItem;
class FcitxPushButtonSettingsItem;
class FcitxKeySettingsItem;
class FcitxIMActivityItem;
} // namespace widgets
} // namespace dcc_fcitx_configtool

namespace Dtk {
namespace Widget {
class DListView;
class DCommandLinkButton;
class DFloatingButton;

} // namespace Widget
} // namespace Dtk

class DBusProvider;
class IMConfig;

class IMSettingWindow : public QWidget
{
    Q_OBJECT
public:
    explicit IMSettingWindow(DBusProvider* dbus, QWidget *parent = nullptr);
    virtual ~IMSettingWindow();
    void updateUI(); //刷新界面
signals:
    void popIMAddWindow(); //弹出添加输入法界面
    void popShortKeyListWindow(const QString &curName, const QStringList &list, QString &name); //弹出快捷键冲突界面
    void popShortKeyStrWindow(const QString &curName, const QString &str, QString &name); //弹出快捷键冲突界面
    void availWidgetAdd(const FcitxQtInputMethodItem &item);
    void requestNextPage(QWidget * const w) const;
private:
    void initUI(); //初始化界面
    void initConnect(); //初始化信号槽
    void readConfig(); //读取配置文件
    void itemSwap(FcitxQtInputMethodItem* item, const bool &isUp = true);
private slots:
    void onEditBtnClicked(const bool &flag); //启用编辑
    void onCurIMChanged(FcitxQtInputMethodItemList* list);
    void onAddBtnCilcked();
    void onItemUp(FcitxQtInputMethodItem* item);
    void onItemDown(FcitxQtInputMethodItem* item);
    void onItemDelete(FcitxQtInputMethodItem* item);
    void doReloadConfigUI();
    void onReloadConnect();
private:
    dcc_fcitx_configtool::widgets::FcitxSettingsGroup *m_IMListGroup {nullptr}; //输入法列表容器
    dcc_fcitx_configtool::widgets::FcitxSettingsGroup *m_shortcutGroup {nullptr}; //输入法快捷键容器
    dcc_fcitx_configtool::widgets::FcitxSettingsHead *m_editHead {nullptr}; //编辑按钮
    dcc_fcitx_configtool::widgets::FcitxComBoboxSettingsItem *m_imSwitchCbox {nullptr}; //切换输入法（快捷键）
    dcc_fcitx_configtool::widgets::FcitxKeySettingsItem *m_defaultIMKey {nullptr}; //默认输入法（快捷键）
    QPushButton* m_advSetKey  {nullptr}; //高级设置
    Dtk::Widget::DCommandLinkButton* m_resetBtn {nullptr}; //重置输入法（快捷键）
    Dtk::Widget::DFloatingButton *m_addIMBtn {nullptr}; //添加输入法
    QVBoxLayout *m_mainLayout;
    DBusProvider* m_dbus;
    IMConfig *m_config;
};

#endif // IMSETTINGWINDOW_H
