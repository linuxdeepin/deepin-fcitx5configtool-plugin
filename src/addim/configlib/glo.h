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

QString getEnglishLanguageName(QString uniqueName);

void setUseIMList(const FcitxQtStringKeyValueList& useIMs);

FcitxQtStringKeyValueList& getUseIMList();

void setUseIMLanguageCount(int count);
int getUseIMLanguageCount();

void setCurrentIMViewIndex(int index);
int getCurrentIMViewIndex();

#endif
