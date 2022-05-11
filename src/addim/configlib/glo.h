/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef ADDIM_GLO_H
#define ADDIM_GLO_H

#include "fcitxqtdbustypes.h"

#define DTK_VERSION_MAJOR    5

using namespace fcitx;

int initCategroyLanguageMap();

QString buildCategroyLanguageName(QString chineseLanguage);

QString getEnglishLanguageName(QString chineseLanguage);
QString getChineseLanguageName(QString englishLanguage);

void setUseIMList(const FcitxQtStringKeyValueList& useIMs);

FcitxQtStringKeyValueList& getUseIMList();

void setMaxUseIMLanguageIndex(int index);
int getMaxUseIMLanguageIndex();

#endif
