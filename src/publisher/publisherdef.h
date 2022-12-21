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
#ifndef PUBLISHERDEF_H
#define PUBLISHERDEF_H

#include <QString>

#define DeleteObject_Null(obj) \
    if (obj) \
        delete obj; \
    obj = nullptr;

#define for_int(count) for (int i = 0; i < count; ++i)

#define Dynamic_Cast(Type, obj, base) \
    Type *obj = dynamic_cast<Type *>(base)

#define Dynamic_Cast_CheckNull(Type, obj, base) \
    Type *obj = dynamic_cast<Type *>(base); \
    if (!obj) \
        return;

typedef   class FcitxQtInputMethodItem {
public:
    QString name() const {return m_name;}
    QString uniqueName() const {return m_uniqueName;}
    QString languageCode() const {return m_langcode;}
    bool configurable() const {return  m_configurable;}
    bool enabled() const {return m_enabled;}

    void setName(const QString& name)  { m_name = name;}
    void setUniqueName(const QString& uniqueName)  { m_uniqueName = uniqueName;}
    void setLanguageCode(const QString& langcode)  { m_langcode  = langcode;}
    void setConfigurable(bool configurable)  {  m_configurable = configurable;}
    void setEnabled(bool enabled)  { m_enabled = enabled;}
private:
    QString m_name;
    QString m_uniqueName;
    QString m_langcode;
    bool m_enabled;
    bool m_configurable;
} FcitxQtInputMethodItem;

typedef  QList<FcitxQtInputMethodItem*> FcitxQtInputMethodItemList;

#endif // PUBLISHERDEF_H
