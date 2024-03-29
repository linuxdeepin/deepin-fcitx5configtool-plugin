// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fcitx5configplugin.h"
#include "interface/moduleobject.h"
#include "interface/pagemodule.h"

#include <QLabel>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <QCoreApplication>
#include <QTranslator>
#include <QLocale>
#include "window/imwindow.h"

Fcitx5ConfigPlugin::Fcitx5ConfigPlugin()
{
}

QString Fcitx5ConfigPlugin::name() const
{
    return tr("deepin-fcitx5configtool-plugin");
}

QString Fcitx5ConfigPlugin::follow() const
{
    return "keyboard";
}

QString Fcitx5ConfigPlugin::location() const
{
    return QString("2");
}

ModuleObject* Fcitx5ConfigPlugin::module()
{
    ModuleObject *module = new PageModule(QString("Manage Input Methods"), tr("Input Method"), this);
    LabelModule *labelModule = new LabelModule();
    module->appendChild(labelModule);

    return module;
}



QWidget *LabelModule::page()
{
    IMWindow *m_imWindow = new IMWindow();
    return m_imWindow;
}

