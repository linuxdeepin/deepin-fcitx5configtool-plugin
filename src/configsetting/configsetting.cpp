// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configsetting.h"

#include <qsettingbackend.h>
#include <dsettings.h>
#include <dsettingsoption.h>
#include <DSettingsWidgetFactory>

#include <QStandardPaths>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>
#include <DConfig>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

ConfigSettings::ConfigSettings(QObject *parent)
    : QObject(parent)
    , m_dconfig(DConfig::create("org.deepin.fcitx5.configtoolplugin", "org.deepin.fcitx5.configtoolplugin", QString(), this))
{
}

ConfigSettings::~ConfigSettings()
{
    delete m_dconfig;
}


WindowState ConfigSettings::GetKeyValue(const QString &key)
{
    int state = -1;
    if (m_dconfig->isValid() && m_dconfig->keyList().contains(key)){
        state = m_dconfig->value(key).toInt();
    }
    return (WindowState)state;
}