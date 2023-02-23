// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMSETTINGWINDOW_H
#define IMSETTINGWINDOW_H

#include "publisher/publisherdef.h"

#include <widgets/comboxwidget.h>
#include <widgets/settingsgroup.h>
#include <widgets/settingshead.h>

#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace dcc_fcitx_configtool {
namespace widgets {
class FcitxSettingsGroup;
class FcitxPushButtonSettingsItem;
class FcitxKeySettingsItem;
} // namespace widgets
} // namespace dcc_fcitx_configtool

namespace Dtk {
namespace Widget {
class DListView;
class DCommandLinkButton;
class DFloatingButton;
class DIconButton;
} // namespace Widget
} // namespace Dtk

class DBusProvider;
class IMConfig;
class AdvanceConfig;
class ConfigSettings;
class QStandardItemModel;
class FilteredIMModel;

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
    void initWindows();

    void setResetButtonEnable();
    void hideResetButton();
    void setSwitchFirstEnable();
    void hideSwitchFirstButton();
    void setAdvanceButtonEnable();
    void hideAdvanceButton();
    void setSwitchFirstFuncEnable();
private slots:
    void onCurIMChanged(FilteredIMModel* model);
    void onAddBtnCilcked();
    void onItemUp(int row);
    void onItemDown(int row);
    void onItemDelete(int row);
    void onItemConfig(int row);
    void doReloadConfigUI();
    void onReloadConnect();
private:
    Dtk::Widget::DListView *m_IMListGroup; //输入法列表容器
    QStandardItemModel *m_IMListModel;
    DCC_NAMESPACE::SettingsGroup *m_shortcutGroup; //输入法快捷键容器
    DCC_NAMESPACE::SettingsHead *m_editHead;       //编辑按钮
    Dtk::Widget::DIconButton *m_deleteBtn;
    DCC_NAMESPACE::ComboxWidget *m_imSwitchCbox;   //切换输入法（快捷键）
    dcc_fcitx_configtool::widgets::FcitxKeySettingsItem *m_defaultIMKey {nullptr}; //默认输入法（快捷键）
    QPushButton* m_advSetKey  {nullptr}; //高级设置
    Dtk::Widget::DCommandLinkButton* m_resetBtn {nullptr}; //重置输入法（快捷键）
    Dtk::Widget::DFloatingButton *m_addIMBtn {nullptr}; //添加输入法
    QVBoxLayout *m_mainLayout;
    DBusProvider* m_dbus;
    IMConfig *m_config;
    AdvanceConfig *m_advanceConfig;
    ConfigSettings *m_setting;
};

#endif // IMSETTINGWINDOW_H
