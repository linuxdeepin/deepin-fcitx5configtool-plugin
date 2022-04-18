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
#ifndef IMCONFIG_H
#define IMCONFIG_H
class QString;
class QStringList;

//默认输入法 切换方式 默认输入法快捷键
class IMConfig
{
public:
    static QString IMSwitchKey();
    static bool setIMSwitchKey(const QString &);
    static QString defaultIMKey();
    static bool setDefaultIMKey(const QString &);
    static QString IMPluginKey(const QString &);
    static QString IMPluginPar(const QString &);

    static bool checkShortKey(const QStringList &str, QString &configName);
    static bool checkShortKey(const QString &str, QString &configName);
    static bool setIMvalue(const QString &key, const QString &value);
private:
    static QString configFile(const QString &filePath, const QString &key);
    static QString configFile(const QString &filePath, const QString &group, const QString &key);
    static bool setConfigFile(const QString &filePath, const QString &key, const QString &value);
    static QString prefix;
};

#endif // IMCONFIG_H
