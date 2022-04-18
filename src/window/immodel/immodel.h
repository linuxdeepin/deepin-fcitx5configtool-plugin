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
#ifndef IMMODEL_H
#define IMMODEL_H

#include <QObject>
#include <fcitxqtinputmethoditem.h>

namespace Dtk {
namespace Widget {
class DStandardItem;
} // namespace Widget
} // namespace Dtk

bool operator==(const FcitxQtInputMethodItem &item, const FcitxQtInputMethodItem &item2);

class IMModel : public QObject
{
    Q_OBJECT
private:
    explicit IMModel();
    IMModel(const IMModel &tmp) = delete;
    IMModel operator=(const IMModel &tmp) = delete;
    virtual ~IMModel();

public:
    //单例
    static IMModel *instance();
    static void deleteIMModel();

    //self
    void setEdit(bool flag); //设置编辑状态
    bool isEdit() { return m_isEdit; } //获取编辑状态
    int getIMIndex(const QString &IM) const;
    int getIMIndex(const FcitxQtInputMethodItem &IM) const;
    FcitxQtInputMethodItem getIM(const int &index) const;
    const FcitxQtInputMethodItemList &getAvailIMList() const { return m_availeIMList; }
    const FcitxQtInputMethodItemList &getCurIMList() const { return m_curIMList; }

public slots:
    void onUpdateIMList(); //更新输入法列表
    void onAddIMItem(FcitxQtInputMethodItem item); //添加输入法
    void onDeleteItem(FcitxQtInputMethodItem item); //删除item
    void onItemUp(FcitxQtInputMethodItem item); //item上移
    void onItemDown(FcitxQtInputMethodItem item); //item下移
    void switchPoistion(FcitxQtInputMethodItem item, int dest);
    void onConfigShow(const FcitxQtInputMethodItem &item); //显示输入法设置界面
    void addIMItem();

signals:
    void availIMListChanged(FcitxQtInputMethodItemList);
    void curIMListChanaged(FcitxQtInputMethodItemList);
    void IMItemSawp(int index, int index2);

private:
    void IMListSave(); //保存输入法列表至fcitx

private:
    static IMModel *m_ins;
    FcitxQtInputMethodItemList m_curIMList; //当前使用输入法
    FcitxQtInputMethodItemList m_availeIMList; //当前未使用输入法
    bool m_isEdit {false}; //编辑状态
};

#endif // IMMODEL_H
