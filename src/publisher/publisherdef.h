// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
