// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ADDIM_GLO_H
#define ADDIM_GLO_H

#include "fcitxqtdbustypes.h"

#define DTK_VERSION_MAJOR    5

using namespace fcitx;

void setUseIMList(const FcitxQtStringKeyValueList& useIMs);

FcitxQtStringKeyValueList& getUseIMList();

void setUseIMLanguageCount(int count);
int getUseIMLanguageCount();

void setCurrentIMViewIndex(int index);
int getCurrentIMViewIndex();

#endif
