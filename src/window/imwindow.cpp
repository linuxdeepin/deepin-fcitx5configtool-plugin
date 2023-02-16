// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imwindow.h"
#include "imsettingwindow.h"
#include "publisher/publisherdef.h"
#include "fcitx5Interface/dbusprovider.h"
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QTranslator>
#include <libintl.h>
#include <QToolButton>
#include <QComboBox>
#include <QAccessible>
#include <QAccessibleInterface>

#include "widgets/titlelabel.h"
#include <DWidget>
#include "configsetting/configsetting.h"

IMWindow::IMWindow(QWidget *parent)
    : QWidget(parent)
{
    initFcitxInterface();
    initUI();
    initConnect();
}

IMWindow::~IMWindow()
{
}

void IMWindow::initFcitxInterface()
{
}

void IMWindow::initUI()
{
    ConfigSettings *conf = new ConfigSettings(this);
    DBusProvider *dbus = new DBusProvider(this);
    m_stackedWidget = new QStackedWidget(this);
    m_settingWindow = new IMSettingWindow(dbus, this);
    qInfo() << "load setting window";
    connect(m_settingWindow, &IMSettingWindow::requestNextPage, this, &IMWindow::requestNextPage);
   // m_addWindow = new IMAddWindow(this);
   // m_addWindow->setAccessibleName("addWindow");
    m_stackedWidget->addWidget(m_settingWindow);
   // m_stackedWidget->addWidget(m_addWindow);
    m_stackedWidget->setCurrentIndex(0);
    //界面布局
    m_pLayout = new QVBoxLayout(this);
    m_pLayout->addWidget(m_stackedWidget);
    m_pLayout->setMargin(0);
    m_pLayout->setSpacing(0);
}

void IMWindow::initConnect()
{
    auto func = [ = ]() {
        m_stackedWidget->setCurrentIndex(PopIMSettingWindow);
        m_settingWindow->updateUI();
    };
//    connect(m_addWindow, &IMAddWindow::popSettingsWindow, func);
//    connect(m_settingWindow, &IMSettingWindow::availWidgetAdd, m_addWindow, &IMAddWindow::pushItemAvailwidget);
//    connect(m_settingWindow, &IMSettingWindow::popIMAddWindow, [ = ]() {
//        m_stackedWidget->setCurrentIndex(PopIMAddWindow);
//        m_addWindow->updateUI();
//    });

    connect(m_settingWindow, &IMSettingWindow::popShortKeyListWindow, [ = ](const QString & curName, const QStringList & list, QString & name) {
        QString tmpString;
        for (const QString &key : list) {
            tmpString = (key != list.last())? key + "+": key;
        }

        m_stackedWidget->setCurrentIndex(PopShortcutKeyWindow);
        m_settingWindow->updateUI();
    });
}
