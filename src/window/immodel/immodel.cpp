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
#include "immodel.h"
#include "fcitxInterface/global.h"
#include "publisher/publisherdef.h"
#include "widgets/imactivityitem.h"
#include "imconfig.h"
#include <QApplication>
#include <QProcess>

using namespace Fcitx;
using namespace Dtk::Widget;

IMModel *IMModel::m_ins {nullptr};

bool operator==(const FcitxQtInputMethodItem &item, const FcitxQtInputMethodItem &item2)
{
    return item.name() == item2.name()
           && item.langCode() == item2.langCode()
           && item.uniqueName() == item2.uniqueName() && item.enabled() == item2.enabled();
}

IMModel::IMModel()
    : QObject(nullptr)
{
    onUpdateIMList();
    connect(Global::instance(), &Global::connectStatusChanged, this, &IMModel::onUpdateIMList);
}

IMModel::~IMModel()
{
}

IMModel *IMModel::instance()
{
    return m_ins == nullptr ? (m_ins = new IMModel()) : m_ins;
}

void IMModel::deleteIMModel()
{
    m_ins->IMListSave();
    DeleteObject_Null(m_ins);
}

void IMModel::setEdit(bool flag)
{
    m_isEdit = flag;
    if (!m_isEdit) {
        IMListSave();
        QTimer::singleShot(200, this, SLOT(addIMItem()));
    }
}

int IMModel::getIMIndex(const QString &IM) const
{
    if (IM.isEmpty()) {
        return -1;
    }

    for (int i = 0; i < m_curIMList.count(); ++i) {
        if (m_curIMList[i].name() == IM
                || m_curIMList[i].uniqueName().indexOf(IM, Qt::CaseInsensitive) != -1
                || m_curIMList[i].langCode().indexOf(IM, Qt::CaseInsensitive) != -1) {
            return i;
        }
    }
    return -1;
}

int IMModel::getIMIndex(const FcitxQtInputMethodItem &IM) const
{
    return getIMIndex(IM.name());
}

FcitxQtInputMethodItem IMModel::getIM(const int &index) const
{
    if (index > m_curIMList.count() || index < 0)
        return FcitxQtInputMethodItem();
    return m_curIMList[index];
}

void IMModel::onUpdateIMList()
{
    if (Global::instance()->inputMethodProxy()) {
        FcitxQtInputMethodItemList &&list = Global::instance()->inputMethodProxy()->iMList();
        FcitxQtInputMethodItemList curList, availList;

        Q_FOREACH (const FcitxQtInputMethodItem &im, list) {
            im.enabled() ? curList.append(im) : availList.append(im);
        }

        if (curList != m_curIMList) {
            m_curIMList.swap(curList);
            emit curIMListChanaged(m_curIMList);
        }

        if (availList != m_availeIMList) {
            m_availeIMList.swap(availList);
            emit availIMListChanged(m_availeIMList);
        }

    } else {
        m_availeIMList.clear();
        m_curIMList.clear();
        emit curIMListChanaged(m_curIMList);
        emit availIMListChanged(m_availeIMList);
    }
}

void IMModel::onAddIMItem(FcitxQtInputMethodItem item)
{
    if (item.name().isEmpty() || item.uniqueName().isEmpty())
        return;

    m_availeIMList.removeAll(item);
    item.setEnabled(true);
    m_curIMList.insert(1, item);
    IMListSave();
    emit curIMListChanaged(m_curIMList);
    QTimer::singleShot(200, this, SLOT(addIMItem()));

}

void IMModel::addIMItem()
{
    emit availIMListChanged(m_availeIMList);
}

void IMModel::onDeleteItem(FcitxQtInputMethodItem item)
{
    m_curIMList.removeAll(item);
    item.setEnabled(false);
    m_availeIMList.append(item);
    IMListSave();
}

void IMModel::onItemUp(FcitxQtInputMethodItem item)
{
    int row = getIMIndex(item);
    m_curIMList.swap(row, row - 1);
    IMListSave();
    emit IMItemSawp(row, row - 1);
}

void IMModel::onItemDown(FcitxQtInputMethodItem item)
{
    int row = getIMIndex(item);

    if (row == m_curIMList.count() - 1) {
        return;
    }

    m_curIMList.swap(row, row + 1);
    IMListSave();
    emit IMItemSawp(row, row + 1);
}

void IMModel::switchPoistion(FcitxQtInputMethodItem item, int dest)
{
    m_curIMList.removeOne(item);
    m_curIMList.insert(dest, item);
    IMListSave();
}

void IMModel::onConfigShow(const FcitxQtInputMethodItem &item)
{
    QString imName = item.name();
    QString imLangCode = item.langCode();
    QString imUniqueName = item.uniqueName();

    QStringList closeSrcImList {
        "chineseime", "iflyime", "sogoupinyin", "baidupinyin"};

    if (closeSrcImList.contains(imUniqueName)) {
        QProcess::startDetached("sh -c " + IMConfig::IMPluginKey(imUniqueName));
    } else if (imUniqueName.compare("huayupy") == 0) {
        QProcess::startDetached("sh -c \"" + IMConfig::IMPluginKey(imUniqueName) + " " + IMConfig::IMPluginPar(imUniqueName)+"\"");
    } else {
        QDBusPendingReply<QString>
        result = Global::instance()->inputMethodProxy()->GetIMAddon(imUniqueName);
        result.waitForFinished();
        if (result.isValid()) {
            QProcess::startDetached("sh -c \"fcitx-config-gtk3 " + result.value()+"\"");
        }
    }
}

void IMModel::IMListSave()
{
    if (Global::instance()->inputMethodProxy()) {
        FcitxQtInputMethodItemList &&list = (m_curIMList + m_availeIMList);
        if (list != Global::instance()->inputMethodProxy()->iMList()) {
            if (Global::instance()->inputMethodProxy()) {
                Global::instance()->inputMethodProxy()->setIMList(list);
                Global::instance()->inputMethodProxy()->ReloadConfig();
            }
        }
    }
}
