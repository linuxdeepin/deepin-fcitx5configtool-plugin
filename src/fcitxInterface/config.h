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
#define VERSION_STRING_FULL "0.5.4"
#define DATADIR "/usr/share"
#define LOCALEDIR "/usr/share/locale"
#define EXEC_PREFIX "/usr/bin"
#define PACKAGE "fcitx"
#define XLIBDIR "/usr/lib/X11"
#define FCITX4_EXEC_PREFIX "usr"
#define FCITX4_MAJOR_VERSION 4
#define FCITX4_MINOR_VERSION 2
#define FCITX4_PATCH_VERSION 9

#define FCITX_CHECK_VERSION(major, minor, micro) \
    (FCITX4_MAJOR_VERSION > (major) || (FCITX4_MAJOR_VERSION == (major) && FCITX4_MINOR_VERSION > (minor)) || (FCITX4_MAJOR_VERSION == (major) && FCITX4_MINOR_VERSION == (minor) && FCITX4_PATCH_VERSION >= (micro)))
