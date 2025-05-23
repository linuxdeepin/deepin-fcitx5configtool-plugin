// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "imlistmodel.h"

#include <model.h>

namespace deepin {
namespace fcitx5configtool {

IMListModel::IMListModel(QObject *parent)
    : QAbstractListModel(parent), m_imModel(nullptr)
{
    qDebug() << "IMListModel created";
}

IMListModel::~IMListModel()
{
}

void IMListModel::resetData(fcitx::kcm::FilteredIMModel *model)
{
    qDebug() << "Resetting IM list data";
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
    qDebug() << "IM list data reset complete, item count:" << m_items.count();
}

int IMListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.count();
}

QVariant IMListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.count())
        return QVariant();

    const IMItem &item = m_items.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
    case IMRoles::NameRole:
        return item.name;
    case IMRoles::UniqueName:
        return item.uniqueName;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> IMListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[UniqueName] = "uniqueName";
    return roles;
}

int IMListModel::count() const
{
    return rowCount();
}
bool IMListModel::canMoveUp(int index) const
{
    return index > 0 && index < m_items.count();
}

bool IMListModel::canMoveDown(int index) const
{
    return index >= 0 && index < m_items.count() - 1;
}

void IMListModel::removeItem(int index)
{
    qDebug() << "Removing IM item at index:" << index;
    if (index < 0 || index >= m_items.count()) {
        qWarning() << "Invalid remove index:" << index;
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_items.removeAt(index);
    endRemoveRows();

    Q_EMIT requestRemove(index);
}

void IMListModel::moveItem(int from, int to)
{
    qDebug() << "Moving IM item from" << from << "to" << to;
    if (from < 0 || from >= m_items.count() || to < 0 || to >= m_items.count()) {
        qWarning() << "Invalid move positions:" << from << "->" << to;
        return;
    }

    if (!beginMoveRows(QModelIndex(), from, from, QModelIndex(), to > from ? to + 1 : to))
        return;

    m_items.move(from, to);
    endMoveRows();

    Q_EMIT requestMove(from, to);
}

bool IMListModel::canRemove() const
{
    return m_items.count() > 1;
}

}   // namespace fcitx5configtool
}   // namespace deepin
