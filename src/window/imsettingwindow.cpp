// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imsettingwindow.h"

#include "addim/widgetslib/impage.h"
#include "model.h"
#include "configsetting/configsetting.h"
#include "fcitx5Interface/advanceconfig.h"
#include "fcitx5Interface/configwidgetslib/configwidget.h"
#include "fcitx5Interface/dbusprovider.h"
#include "fcitx5Interface/imconfig.h"
#include "publisher/publisherdef.h"
#include "widgets/contentwidget.h"
#include "widgets/keysettingsitem.h"

#include <libintl.h>
#include <widgets/settingsgroup.h>
#include <widgets/settingshead.h>
#include <widgets/settingsitem.h>
#include <widgets/titlelabel.h>

#include <DCommandLinkButton>
#include <DFloatingButton>
#include <DFontSizeManager>
#include <DListView>
#include <DStandardItem>
#include <DWidgetUtil>

#include <QEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QProcess>

DWIDGET_USE_NAMESPACE

using namespace dcc_fcitx_configtool::widgets;

class EventConsumer : public QEventLoop
{
public:
    explicit EventConsumer(QObject *producer = qApp, QObject *parent = nullptr);
    ~EventConsumer() override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QObject *m_producer;
};

EventConsumer::EventConsumer(QObject *producer, QObject *parent)
    : QEventLoop(parent)
    , m_producer(producer)
{
    if (m_producer) {
        m_producer->installEventFilter(this);
    }
}

EventConsumer::~EventConsumer()
{
    if (m_producer) {
        m_producer->removeEventFilter(this);
    }
}

bool EventConsumer::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)
    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        return true;
    default:
        return false;
    }
}

IMSettingWindow::IMSettingWindow(DBusProvider *dbus, QWidget *parent)
    : QWidget(parent)
    , m_dbus(dbus)
    , m_config(new IMConfig(dbus, IMConfig::Tree, this))
    , m_advanceConfig(new AdvanceConfig("fcitx://config/global", m_dbus, this))
    , m_setting(new ConfigSettings())
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
    //滑动窗口
    m_mainLayout = new QVBoxLayout();

    QVBoxLayout *scrollAreaLayout = new QVBoxLayout(this);
    scrollAreaLayout->setContentsMargins(10, 0, 10, 0);
    scrollAreaLayout->setSpacing(0);

    QHBoxLayout *imHeaderLayout = new QHBoxLayout();

    //创建标题
    m_editHead = new DCC_NAMESPACE::SettingsHead();
    m_editHead->setEditEnable(false);
    m_editHead->setTitle(tr("Manage Input Methods"));
    m_editHead->layout()->setContentsMargins(10, 4, 10, 0);
    m_deleteBtn = new DIconButton(DStyle::SP_DecreaseElement);
    m_deleteBtn->setEnabled(false);
    auto *addBtn = new DIconButton(DStyle::SP_IncreaseElement);
    connect(m_deleteBtn, &DIconButton::clicked, this, [=]() {
        qInfo() << "user clicked delete button";
        int row = m_IMListGroup->currentIndex().row();
        onItemDelete(row);
    });

    imHeaderLayout->addWidget(m_editHead);
    imHeaderLayout->addStretch();
    imHeaderLayout->addWidget(m_deleteBtn);
    imHeaderLayout->addWidget(addBtn);

    connect(addBtn, &DIconButton::clicked, this, [=]() {
        qInfo() << "user clicked add button";
        fcitx::addim::IMPage mainWindow(m_dbus, m_config, this);
        Dtk::Widget::moveToCenter(&mainWindow);
        mainWindow.exec();
    });

    //输入法管理 编辑按钮
    m_IMListGroup = new DListView(this);
    m_IMListGroup->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_IMListGroup->setBackgroundType(DStyledItemDelegate::BackgroundType::ClipCornerBackground);
    // m_IMListGroup->setSwitchAble(true);
    m_IMListGroup->setSpacing(0);
    m_IMListGroup->setAttribute(Qt::WA_Hover, true);
    m_IMListGroup->installEventFilter(this);

    m_IMListModel = new QStandardItemModel(this);
    m_IMListGroup->setModel(m_IMListModel);

    onCurIMChanged(m_config->currentIMModel());

    connect(m_IMListGroup->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this,
            [this](const QModelIndex &current, [[maybe_unused]] const QModelIndex &previous) {
                m_deleteBtn->setEnabled(current.isValid());
                QTimer::singleShot(0, [this]() {
                    updateActions();
                });
            });

    QHBoxLayout *shortcutLayout = new QHBoxLayout();
    auto *headTitle = new DCC_NAMESPACE::TitleLabel(tr("Shortcuts"));
    headTitle->setContentsMargins(10, 0, 0, 0);
    DFontSizeManager::instance()->bind(headTitle,
                                       DFontSizeManager::T5,
                                       QFont::DemiBold); // 设置label字体
    m_resetBtn = new DCommandLinkButton(tr("Restore Defaults"), this);
    m_resetBtn->setAccessibleName("Restore Defaults");
    shortcutLayout->addWidget(headTitle);
    shortcutLayout->addStretch();
    shortcutLayout->addWidget(m_resetBtn);

    // 快捷键 切换输入法 切换虚拟键盘 切换至默认输入法
    m_shortcutGroup = new DCC_NAMESPACE::SettingsGroup(nullptr, DCC_NAMESPACE::SettingsGroup::GroupBackground);
    m_imSwitchCbox = new DCC_NAMESPACE::ComboxWidget(tr("Scroll between input methods"));
    m_imSwitchCbox->setComboxOption({ "CTRL_SHIFT", "ALT_SHIFT", "CTRL_SUPER", "ALT_SUPER" });
    m_imSwitchCbox->comboBox()->setAccessibleName("Switch input methods");
    m_defaultIMKey = new FcitxKeySettingsItem(tr("Switch between the current/first input method"));
    m_shortcutGroup->appendItem(m_imSwitchCbox);
    m_shortcutGroup->appendItem(m_defaultIMKey);

    //控件添加至滑动窗口内
    scrollAreaLayout->addLayout(imHeaderLayout);
    scrollAreaLayout->addSpacing(10);
    scrollAreaLayout->addWidget(m_IMListGroup);
    scrollAreaLayout->addSpacing(30);

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
    qInfo() << "read config:" << m_IMListModel->rowCount();
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
        qInfo() << "list changed:" << m_IMListModel->rowCount();
        onCurIMChanged(m_config->currentIMModel());
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
        // 只需要第一组快捷键
        m_defaultIMKey->setList(shortCuts.split(" ").first().split("+"));
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

    connect(m_advSetKey, &QPushButton::clicked, this, [ = ]() {
        QProcess advancedSettingProcess(this);
        advancedSettingProcess.setProgram("fcitx5-config-qt");
        EventConsumer loop;
        advancedSettingProcess.start();
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
        auto finished = QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished);
#else
        auto finished = QOverload<int>::of(&QProcess::finished);
#endif
        connect(&advancedSettingProcess, finished, &loop, &EventConsumer::quit);
        loop.exec();
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

bool IMSettingWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_IMListGroup) {
        switch (event->type()) {
        case QEvent::HoverLeave:
            m_hoveredRow = -1;
            updateActions();
            break;
        case QEvent::HoverMove: {
            auto *he = dynamic_cast<QHoverEvent *>(event);
            int newRow = m_IMListGroup->indexAt(he->pos()).row();
            if (newRow != m_hoveredRow) {
                m_hoveredRow = newRow;
                updateActions();
            }

            break;
        }
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void IMSettingWindow::updateActions() {
    auto selections = m_IMListGroup->selectionModel()->selectedIndexes();

    for (int i = 0; i < m_IMListModel->rowCount(); ++i) {
        auto *item = dynamic_cast<DStandardItem *>(m_IMListModel->item(i));
        auto actions = item->actionList(Qt::Edge::RightEdge);
        if (m_hoveredRow == i || selections.contains(item->index())) {
            for (auto *action : actions) {
                action->setVisible(true);
            }
        } else {
            for (auto *action : actions) {
                action->setVisible(false);
            }
        }
    }
}

//当前输入法列表改变
void IMSettingWindow::onCurIMChanged(FilteredIMModel* model)
{
    m_deleteBtn->setEnabled(false);
    m_IMListModel->clear();

    for (int i = 0; i < model->rowCount(); ++i) {
        QString name = model->index(i).data(Qt::DisplayRole).toString();
        DStandardItem *item = new DStandardItem(name);

        auto *upAction = new DViewItemAction(Qt::AlignVCenter, QSize(), QSize(), true);
        upAction->setIcon(QIcon::fromTheme("arrow_up"));
        upAction->setVisible(false);
        upAction->setDisabled(i == 0);

        auto *downAction =
                new DViewItemAction(Qt::AlignVCenter, QSize(), QSize(), true);
        downAction->setIcon(QIcon::fromTheme("arrow_down"));
        downAction->setVisible(false);
        downAction->setDisabled(i == model->rowCount() - 1);

        auto *configAction =
                new DViewItemAction(Qt::AlignVCenter, QSize(), QSize(), true);
        configAction->setIcon(QIcon::fromTheme("setting"));
        configAction->setVisible(false);

        item->setActionList(Qt::Edge::RightEdge,
                            { upAction, downAction, configAction });

        connect(upAction, &DViewItemAction::triggered, this, [this, i]() {
            onItemUp(i);
        });

        connect(downAction, &DViewItemAction::triggered, this, [this, i]() {
            onItemDown(i);
        });

        connect(configAction, &DViewItemAction::triggered, this, [this, i]() {
            onItemConfig(i);
        });

        m_IMListModel->appendRow(item);
        qInfo() << "manager im changed:" << name;
    }
}

void IMSettingWindow::onItemUp(int row)
{
    m_config->move(row, row - 1);
    m_config->save();
    m_IMListGroup->setCurrentIndex(m_IMListModel->index(row - 1, 0));
}

void IMSettingWindow::onItemDown(int row)
{
    m_config->move(row, row + 1);
    m_config->save();
    m_IMListGroup->setCurrentIndex(m_IMListModel->index(row + 1, 0));
}

void IMSettingWindow::onItemConfig(int row)
{
    auto item = m_config->currentIMModel()->index(row);
    QString uniqueName = item.data(FcitxIMUniqueNameRole).toString();
    QString title = item.data(Qt::DisplayRole).toString();
    QPointer<QDialog> dialog = ConfigWidget::configDialog(
        this, m_dbus, QString("fcitx://config/inputmethod/%1").arg(uniqueName),
        title);
    dialog->exec();
    delete dialog;
}

void IMSettingWindow::onItemDelete(int row)
{
    m_IMListModel->removeRow(row);
    m_config->removeIM(row);
    m_config->save();
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
