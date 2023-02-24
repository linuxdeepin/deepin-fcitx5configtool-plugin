// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#include "glo.h"

#include <DPinyin>

#include <QDebug>
#include <QLocale>
#include "xkbrules.h"

DCORE_USE_NAMESPACE

QMap<QString, QString> g_uniqueNameToEnglishMap;
QMap<QString, QString> g_uniqueNameToChineseMap;

FcitxQtStringKeyValueList g_useIMList;
int g_useLanguageCount = 0;

int g_currentIMViewIndex = 0;

int g_initCategroyLanguageFlag = false;

void setUseIMList(const FcitxQtStringKeyValueList& useIMs)
{
    g_useIMList = useIMs;
}

FcitxQtStringKeyValueList& getUseIMList()
{
    return g_useIMList;
}

void setUseIMLanguageCount(int count)
{
    g_useLanguageCount = count;
}

int getUseIMLanguageCount()
{
    return g_useLanguageCount;
}

void setCurrentIMViewIndex(int index)
{
    g_currentIMViewIndex = index;
}

int getCurrentIMViewIndex()
{
    return g_currentIMViewIndex;
}
