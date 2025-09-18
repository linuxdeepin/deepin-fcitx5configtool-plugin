// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "imlistmodel.h"

#include <model.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(imListModel, "fcitx5.configtool.imlistmodel")

namespace deepin {
namespace fcitx5configtool {

IMListModel::IMListModel(QObject *parent)
    : QAbstractListModel(parent), m_imModel(nullptr)
{
    qCDebug(imListModel) << "Entering IMListModel constructor";
}

IMListModel::~IMListModel()
{
    qCDebug(imListModel) << "Entering IMListModel destructor";
}

void IMListModel::resetData(fcitx::kcm::FilteredIMModel *model)
{
    qCDebug(imListModel) << "Entering resetData";
    beginResetModel();
    m_items.clear();
    m_imModel = model;

    if (m_imModel) {
        for (int i = 0; i < m_imModel->rowCount(); i++) {
            IMItem item;
            item.name = m_imModel->data(m_imModel->index(i, 0), Qt::DisplayRole).toString();
            item.uniqueName = m_imModel->data(m_imModel->index(i, 0), fcitx::kcm::FcitxIMUniqueNameRole).toString();
            m_items.append(item);
        }
    }

    endResetModel();
    qCDebug(imListModel) << "IM list data reset complete, item count:" << m_items.count();
}

int IMListModel::rowCount(const QModelIndex &parent) const
{
    // qCDebug(imListModel) << "Entering rowCount";
    if (parent.isValid())
        return 0;

    // qCDebug(imListModel) << "Exiting rowCount";
    return m_items.count();
}

QVariant IMListModel::data(const QModelIndex &index, int role) const
{
    // qCDebug(imListModel) << "Entering data";
    if (!index.isValid() || index.row() >= m_items.count())
        return QVariant();

    const IMItem &item = m_items.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
    case IMRoles::NameRole:
        // qCDebug(imListModel) << "role case NameRole";
        return item.name;
    case IMRoles::UniqueName:
        // qCDebug(imListModel) << "role case UniqueName";
        return item.uniqueName;
    default:
        return QVariant();
    }
    // qCDebug(imListModel) << "Exiting data";
}

QHash<int, QByteArray> IMListModel::roleNames() const
{
    // qCDebug(imListModel) << "Entering roleNames";
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[UniqueName] = "uniqueName";
    // qCDebug(imListModel) << "Exiting roleNames";
    return roles;
}

int IMListModel::count() const
{
    // qCDebug(imListModel) << "Entering count";
    return rowCount();
}
bool IMListModel::canMoveUp(int index) const
{
    // qCDebug(imListModel) << "Entering canMoveUp for index" << index;
    return index > 0 && index < m_items.count();
}

bool IMListModel::canMoveDown(int index) const
{
    // qCDebug(imListModel) << "Entering canMoveDown for index" << index;
    return index >= 0 && index < m_items.count() - 1;
}

void IMListModel::removeItem(int index)
{
    qCDebug(imListModel) << "Removing IM item at index:" << index;
    if (index < 0 || index >= m_items.count()) {
        qCWarning(imListModel) << "Invalid remove index:" << index;
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_items.removeAt(index);
    endRemoveRows();

    Q_EMIT requestRemove(index);
    qCDebug(imListModel) << "Exiting removeItem";
}

void IMListModel::moveItem(int from, int to)
{
    qCDebug(imListModel) << "Moving IM item from" << from << "to" << to;
    if (from < 0 || from >= m_items.count() || to < 0 || to >= m_items.count()) {
        qCWarning(imListModel) << "Invalid move positions:" << from << "->" << to;
        return;
    }

    if (!beginMoveRows(QModelIndex(), from, from, QModelIndex(), to > from ? to + 1 : to))
        return;

    m_items.move(from, to);
    endMoveRows();

    Q_EMIT requestMove(from, to);
    qCDebug(imListModel) << "Exiting moveItem";
}

bool IMListModel::canRemove() const
{
    // qCDebug(imListModel) << "Entering canRemove";
    return m_items.count() > 1;
}

}   // namespace fcitx5configtool
}   // namespace deepin
