// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _CONFIGSETTING_H_
#define _CONFIGSETTING_H_

#include "settingsdef.h"

#include <QJsonObject>
#include <QWidget>
#include "dtkcore_global.h"
DCORE_BEGIN_NAMESPACE
class DConfig;
DCORE_END_NAMESPACE

class ConfigSettings : public QObject
{
    Q_OBJECT
public:
    ConfigSettings(QObject *parent = nullptr);
    ~ConfigSettings();

    WindowState GetKeyValue(const QString &key);
private:
    DTK_CORE_NAMESPACE::DConfig *m_dconfig;
};

#endif