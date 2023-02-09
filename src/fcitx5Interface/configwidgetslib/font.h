// SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _CONFIGLIB_FONT_H_
#define _CONFIGLIB_FONT_H_

#include <QFont>
#include <QString>


QFont parseFont(const QString &str);
QString fontToString(const QFont &font);


#endif // _CONFIGLIB_FONT_H_
