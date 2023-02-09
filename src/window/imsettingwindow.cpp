// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imsettingwindow.h"

#include "fcitx5Interface/dbusprovider.h"
#include "widgets/settingsheaderitem.h"
#include "widgets/settingsgroup.h"
#include "widgets/keysettingsitem.h"
#include "widgets/imactivityitem.h"
#include "widgets/settingshead.h"
#include "publisher/publisherdef.h"
#include "fcitx5Interface/imconfig.h"
#include "widgets/contentwidget.h"
#include "addim/widgetslib/addimwindow.h"
#include "fcitx5Interface/advanceconfig.h"
#include "fcitx5Interface/configwidgetslib/configwidget.h"

#include <DWidgetUtil>
#include <DFloatingButton>
#include <DFontSizeManager>
#include <DCommandLinkButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QPushButton>
#include <QEvent>
#include <libintl.h>
#include "configsetting/configsetting.h"

DWIDGET_USE_NAMESPACE

using namespace dcc_fcitx_configtool::widgets;
IMSettingWindow::IMSettingWindow(DBusProvider* dbus, QWidget *parent)
    : QWidget(parent)
    , m_dbus(dbus)
    , m_config(new IMConfig(dbus, IMConfig::Tree, this))
    , m_setting(new ConfigSettings())
    , m_advanceConfig(new AdvanceConfig("fcitx://config/global", m_dbus, this))
{
    initUI();
    initConnect();
    updateUI();
}

IMSettingWindow::~IMSettingWindow()
{
}

void IMSettingWindow::initUI()
{
    //创建标题
    auto newTitleHead = [this](QString str, bool isEdit = false) {
        FcitxSettingsHead *head = new FcitxSettingsHead(isEdit);
        head->setParent(this);
        head->setTitle(str);
        //head->setDeleteEnable(false);
        head->layout()->setContentsMargins(10, 4, 10, 0);
        if (isEdit) {
            m_editHead = head;
            m_editHead->getTitleLabel()->setAccessibleName("Edit");
        }
        connect(head, &FcitxSettingsHead::deleteBtnClicked, this, [=](){
            qInfo() << "user clicked delete button" << endl;
            int index = m_IMListGroup->selectIndex();
            this->onItemDelete(m_config->getFcitxQtInputMethodItemList()->at(index));
        });

        connect(head, &FcitxSettingsHead::addBtnClicked, this, [=]() {
            qInfo() << "user clicked add button" << endl;
            fcitx::addim::AddIMWindow mainWindow(m_dbus, m_config, (DDialog*)this);
            Dtk::Widget::moveToCenter(&mainWindow);
            mainWindow.exec();
        });

        return head;
    };

    m_mainLayout = new QVBoxLayout();
    //滑动窗口

    QVBoxLayout *scrollAreaLayout = new QVBoxLayout(this);
    scrollAreaLayout->setContentsMargins(10, 0, 10, 0);
    scrollAreaLayout->setSpacing(0);

    //输入法管理 编辑按钮
    m_IMListGroup = new FcitxSettingsGroup();
    m_IMListGroup->setSwitchAble(true);
    m_IMListGroup->setSpacing(0);
    onCurIMChanged(m_config->getFcitxQtInputMethodItemList());

    //快捷键 切换输入法 切换虚拟键盘 切换至默认输入法
    m_shortcutGroup = new FcitxSettingsGroup();
    m_shortcutGroup->setSpacing(0);
    m_imSwitchCbox = new FcitxComBoboxSettingsItem(tr("Scroll between input methods"), {"CTRL_SHIFT", "ALT_SHIFT", "CTRL_SUPER", "ALT_SUPER"});
    m_imSwitchCbox->comboBox()->setAccessibleName("Switch input methods");
    m_defaultIMKey = new FcitxKeySettingsItem(tr("Switch between the current/first input method"));
    m_shortcutGroup->appendItem(m_imSwitchCbox);
    m_shortcutGroup->appendItem(m_defaultIMKey);

    //控件添加至滑动窗口内
    scrollAreaLayout->addWidget(newTitleHead(tr("Manage Input Methods"), true));
    scrollAreaLayout->addSpacing(10);
    scrollAreaLayout->addWidget(m_IMListGroup);
    scrollAreaLayout->addSpacing(30);

    //QHBoxLayout 存放m_resetBtn和Shortcuts标题两个控件
    QHBoxLayout *shortcutLayout = new QHBoxLayout();
    QWidget *pWidget = newTitleHead(tr("Shortcuts"));
    shortcutLayout->addWidget(pWidget);

    m_resetBtn = new DCommandLinkButton(tr("Restore Defaults"), this);
    DFontSizeManager::instance()->bind(m_resetBtn, DFontSizeManager::T8, QFont::Normal);
    m_resetBtn->setAccessibleName("Restore Defaults");

    shortcutLayout->addWidget(m_resetBtn, 0, Qt::AlignRight | Qt::AlignBottom);
    scrollAreaLayout->addLayout(shortcutLayout);
    scrollAreaLayout->addSpacing(10);
    scrollAreaLayout->addWidget(m_shortcutGroup);
    scrollAreaLayout->addSpacing(20);

    m_advSetKey = new QPushButton(tr("Advanced Settings"));
    m_advSetKey->setAccessibleName("Advanced Settings");
    m_advSetKey->setMaximumWidth(214);

    scrollAreaLayout->addWidget(m_advSetKey, 0, Qt::AlignHCenter);
    scrollAreaLayout->addStretch();

    //添加至主界面内
    setLayout(scrollAreaLayout);
    qInfo() << "read config:" << m_config->getFcitxQtInputMethodItemList()->count() << endl;
    readConfig();
    initWindows();
}

void IMSettingWindow::onReloadConnect()
{
//    connect(Global::instance()->inputMethodProxy(), &FcitxQtInputMethodProxy::ReloadConfigUI,
//            this, &IMSettingWindow::doReloadConfigUI);
}

void IMSettingWindow::initConnect()
{
    connect(m_config, &IMConfig::imListChanged, this, [=]() {
        qInfo() << "list changed:" << m_config->getFcitxQtInputMethodItemList()->count() << endl;
        onCurIMChanged(m_config->getFcitxQtInputMethodItemList());
    });
    auto reloadFcitx = [ = ](bool flag) {
//        if (Global::instance()->inputMethodProxy() && flag)
//            Global::instance()->inputMethodProxy()->ReloadConfig();
    };
//    connect(Global::instance(), &Global::connectStatusChanged, this, &IMSettingWindow::onReloadConnect);

    connect(m_defaultIMKey, &FcitxKeySettingsItem::editedFinish, [ = ]() {
        m_advanceConfig->switchFirstIMShortCuts(m_defaultIMKey->getKeyToStr());
        m_defaultIMKey->setList(m_defaultIMKey->getKeyToStr().split("_"));
    });

//    onReloadConnect();//    if (IMModel::instance()->isEdit()) {
//        onEditBtnClicked(false);
//    }
//    readConfig();

    connect(m_imSwitchCbox->comboBox(), &QComboBox::currentTextChanged, [ = ]() {
        m_imSwitchCbox->comboBox()->setAccessibleName(m_imSwitchCbox->comboBox()->currentText());
        m_advanceConfig->switchIMShortCuts(m_imSwitchCbox->comboBox()->currentText());
    });

    connect(m_advanceConfig, &AdvanceConfig::switchIMShortCutsChanged, this, [=](const QString& shortCuts) {
        if(shortCuts.contains("Alt")) {
            if(shortCuts.contains("Shift")) {
                m_imSwitchCbox->comboBox()->setCurrentText(("ALT_SHIFT"));
            } else if(shortCuts.contains("Super")) {
                m_imSwitchCbox->comboBox()->setCurrentText(("ALT_SUPER"));
            }
        } if(shortCuts.contains("Control")) {
            if(shortCuts.contains("Shift")) {
                m_imSwitchCbox->comboBox()->setCurrentText(("CTRL_SHIFT"));
            } else if(shortCuts.contains("Super")) {
                m_imSwitchCbox->comboBox()->setCurrentText(("CTRL_SUPER"));
            }
        }
    });

    connect(m_advanceConfig, &AdvanceConfig::switchFirstIMShortCutsChanged, this, [=](const QString& shortCuts) {
        m_defaultIMKey->setList(shortCuts.split("+"));
    });

    connect(m_resetBtn, &QPushButton::clicked, [ = ]() {
        m_advanceConfig->switchFirstIMShortCuts("CTRL_SPACE");
        m_defaultIMKey->setList(QString("CTRL_SPACE").split("_"));
        //保持间隔内不要重新加载
#if defined(USE_MIPS64)
        QTimer::singleShot(200, this, [=](){
#else
        QTimer::singleShot(50, this, [=](){
#endif
            m_advanceConfig->switchIMShortCuts("CTRL_SHIFT");
            m_imSwitchCbox->comboBox()->setCurrentText(("CTRL_SHIFT"));
        });
    });

    connect(m_IMListGroup, &FcitxSettingsGroup::switchPosition, m_config, [=](FcitxQtInputMethodItem* item, int to){
        int from = m_config->getFcitxQtInputMethodItemList()->indexOf(item);
        m_config->move(from , to);
    });
    connect(m_editHead, &FcitxSettingsHead::editChanged, this, &IMSettingWindow::onEditBtnClicked);

    connect(m_advSetKey, &QPushButton::clicked, [ = ]() {
        system("fcitx5-config-qt");
    });
}

void IMSettingWindow::setResetButtonEnable()
{
    auto del = m_setting->GetKeyValue(DCONFIG_SHORTCUT_RESTORE);
    bool enable = (del == WindowState::Disable);
    m_resetBtn->setEnabled(!enable);
}

void IMSettingWindow::hideResetButton()
{
    auto del = m_setting->GetKeyValue(DCONFIG_SHORTCUT_RESTORE);
    bool enable = (del == WindowState::Hide);
    if (enable) {
        m_resetBtn->hide();
    }
}

void IMSettingWindow::setSwitchFirstFuncEnable()
{
    auto del = m_setting->GetKeyValue(DCONFIG_SHORTCUT_SWITCHTORFIRSTFUN);
    bool disbale = (del == WindowState::Disable);
    if (disbale) {
        m_advanceConfig->disableSwitchIMShortCutsFunc(disbale);
        m_advanceConfig->switchIMShortCuts(m_imSwitchCbox->comboBox()->currentText());
        m_advanceConfig->switchFirstIMShortCuts(m_defaultIMKey->getKeyToStr());
    }
}

void IMSettingWindow::setSwitchFirstEnable()
{
    auto del = m_setting->GetKeyValue(DCONFIG_SHORTCUT_SWITCHTORFIRST);
    bool enable = (del == WindowState::Disable);
    m_defaultIMKey->setEnabled(!enable);
    m_imSwitchCbox->setEnabled(!enable);
}

void IMSettingWindow::hideSwitchFirstButton()
{
    auto del = m_setting->GetKeyValue(DCONFIG_SHORTCUT_SWITCHTORFIRST);
    bool enable = (del == WindowState::Hide);
    if (enable) {
        m_defaultIMKey->hide();
        m_imSwitchCbox->hide();
    }
}

void IMSettingWindow::setAdvanceButtonEnable()
{
    auto del = m_setting->GetKeyValue(DCONFIG_ADVANCE_SETTING);
    bool enable = (del == WindowState::Disable);
    m_advSetKey->setEnabled(!enable);
}

void IMSettingWindow::hideAdvanceButton()
{
    auto del = m_setting->GetKeyValue(DCONFIG_ADVANCE_SETTING);
    bool enable = (del == WindowState::Hide);
    if (enable) {
        m_advSetKey->hide();
    }
}

//读取配置文件
void IMSettingWindow::readConfig()
{
//    int index = m_imSwitchCbox->comboBox()->findText(IMConfig::IMSwitchKey());
//    m_imSwitchCbox->comboBox()->setCurrentIndex(index < 0 ? 0 : index);
//    m_defaultIMKey->setList(IMConfig::defaultIMKey().split("_"));
}

void IMSettingWindow::initWindows()
{
    setResetButtonEnable();
    hideResetButton();
    setSwitchFirstEnable();
    hideSwitchFirstButton();
    setAdvanceButtonEnable();
    hideAdvanceButton();
    setSwitchFirstFuncEnable();
}

void IMSettingWindow::updateUI()
{
//    if (IMModel::instance()->isEdit()) {
//        onEditBtnClicked(false);
//    }
//    readConfig();
}

void IMSettingWindow::itemSwap(FcitxQtInputMethodItem* item, const bool &isUp)
{
    int row = m_config->getFcitxQtInputMethodItemList()->indexOf(item);
    Dynamic_Cast_CheckNull(FcitxIMActivityItem, t, m_IMListGroup->getItem(row));

    if (isUp) {
        m_IMListGroup->moveItem(t, row - 1);
        m_config->move(row, row - 1);
    } else {
        if (row == m_config->getFcitxQtInputMethodItemList()->count() - 1) {
            return;
        }
        m_IMListGroup->moveItem(t, row + 1);
        m_config->move(row, row + 1);
    }

    t->setSelectStatus(false, row, m_config->getFcitxQtInputMethodItemList()->count());
    int count = m_IMListGroup->indexOf(t);
    if(count == 0) {
        t->setIndex(FcitxIMActivityItem::firstItem);
    } else if(count == m_IMListGroup->itemCount() -1){
        t->setIndex(FcitxIMActivityItem::lastItem);
    } else {
        t->setIndex(FcitxIMActivityItem::otherItem);
    }
    Dynamic_Cast_CheckNull(FcitxIMActivityItem, t2, m_IMListGroup->getItem(row));
    t2->setSelectStatus(true, row, m_config->getFcitxQtInputMethodItemList()->count());

    int count2 = m_IMListGroup->indexOf(t2);
    if(count2 == 0) {
        t2->setIndex(FcitxIMActivityItem::firstItem);
    } else if(count2 == m_IMListGroup->itemCount() -1){
        t2->setIndex(FcitxIMActivityItem::lastItem);
    } else {
        t2->setIndex(FcitxIMActivityItem::otherItem);
    }
}

//编辑当前输入法列表
void IMSettingWindow::onEditBtnClicked(const bool &flag)
{
//    IMModel::instance()->setEdit(flag);
//    m_IMListGroup->setSwitchAble(!flag);
//    m_editHead->setEdit(flag);
//    for (int i = 0; i < m_IMListGroup->itemCount(); ++i) {
//        Dynamic_Cast(FcitxIMActivityItem, mItem, m_IMListGroup->getItem(i));
//        if (mItem) {
//            mItem->editSwitch(flag);
//        }
//    }
}

//当前输入法列表改变
void IMSettingWindow::onCurIMChanged(FcitxQtInputMethodItemList* list)
{
    m_IMListGroup->clear();

    for (int i = 0; i < list->count(); ++i) {
        FcitxIMActivityItem *tmp = nullptr;
        if (i == 0) {
            if(list->count() == 1) {
                tmp = new FcitxIMActivityItem(list->at(i), FcitxIMActivityItem::onlyoneItem, this);
            } else {
                tmp = new FcitxIMActivityItem(list->at(i), FcitxIMActivityItem::firstItem, this);
            }

        } else if (i == list->count() - 1) {
            tmp = new FcitxIMActivityItem(list->at(i), FcitxIMActivityItem::lastItem, this);
        } else {
            tmp = new FcitxIMActivityItem(list->at(i), FcitxIMActivityItem::otherItem, this);
        }
        connect(tmp, &FcitxIMActivityItem::configBtnClicked, this, &IMSettingWindow::onItemConfig);
        connect(tmp, &FcitxIMActivityItem::upBtnClicked, this, &IMSettingWindow::onItemUp);
        connect(tmp, &FcitxIMActivityItem::downBtnClicked, this, &IMSettingWindow::onItemDown);
        connect(tmp, &FcitxIMActivityItem::deleteBtnClicked, this, &IMSettingWindow::onItemDelete);
        connect(tmp, &FcitxIMActivityItem::selectItem, this, [=](FcitxSettingsItem * item, bool selected){
            Q_UNUSED(item);
            QTimer::singleShot(100, this, [=](){
                m_editHead->setDeleteButtonEnable(selected);
            });
        });
        connect(tmp, &FcitxIMActivityItem::itemSelect, this, [=](bool selected){
            QTimer::singleShot(100, this, [=](){
                m_editHead->setDeleteButtonEnable(selected);
            });
        });
        //tmp->editSwitch(IMModel::instance()->isEdit());
        m_IMListGroup->appendItem(tmp);
        qInfo() << "manager im changed:" << list->at(i)->name() << endl;
        tmp->repaint();
    }
    m_IMListGroup->adjustSize();
}

void IMSettingWindow::onItemUp(FcitxQtInputMethodItem* item)
{
    itemSwap(item, true);
}

void IMSettingWindow::onItemConfig(FcitxQtInputMethodItem* item)
{
    QString uniqueName = item->uniqueName();
    QString title = item->name();
    QPointer<QDialog> dialog = ConfigWidget::configDialog(
        this, m_dbus, QString("fcitx://config/inputmethod/%1").arg(uniqueName),
        title);
    dialog->exec();
    delete dialog;
}

void IMSettingWindow::onItemDown(FcitxQtInputMethodItem* item)
{
    itemSwap(item, false);
}

void IMSettingWindow::onItemDelete(FcitxQtInputMethodItem* item)
{
    int index = m_config->getFcitxQtInputMethodItemList()->indexOf(item);
    Dynamic_Cast_CheckNull(FcitxIMActivityItem, t, m_IMListGroup->getItem(index));

    auto it = m_IMListGroup->getItem(index);
    m_IMListGroup->removeItem(it);
    it->deleteLater();
    m_config->removeIM(index);
}

//添加按钮点击
void IMSettingWindow::onAddBtnCilcked()
{
//    if (IMModel::instance()->isEdit())
//        onEditBtnClicked(false);
//    emit popIMAddWindow();
}

void IMSettingWindow::doReloadConfigUI()
{
//    readConfig();
//    IMModel::instance()->onUpdateIMList();
}
