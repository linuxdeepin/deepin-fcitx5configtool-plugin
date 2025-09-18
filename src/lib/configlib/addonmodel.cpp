/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "addonmodel.h"
#include "logging.h"

#include <QCollator>
#include <QDebug>
#include <fcitx-utils/i18n.h>
#include <fcitx/addoninfo.h>

namespace fcitx {
namespace kcm {
namespace {

QString categoryName(int category) {
    if (category >= 5 || category < 0) {
        return QString();
    }

    const char *str[] = {N_("Input Method"), N_("Frontend"), N_("Loader"),
                         N_("Module"), N_("UI")};

    return _(str[category]);
}

} // namespace

AddonModel::AddonModel(QObject *parent) : CategorizedItemModel(parent) {
    // qCDebug(KCM_FCITX5) << "Entering AddonModel constructor";
}

QVariant AddonModel::dataForCategory(const QModelIndex &index, int role) const {
    // qCDebug(KCM_FCITX5) << "AddonModel::dataForCategory - index:" << index << "role:" << role;
    switch (role) {

    case Qt::DisplayRole:
        return categoryName(addonEntryList_[index.row()].first);

    case CategoryRole:
        return addonEntryList_[index.row()].first;

    case RowTypeRole:
        return CategoryType;

    default:
        return QVariant();
    }
}

QVariant AddonModel::dataForItem(const QModelIndex &index, int role) const {
    // qCDebug(KCM_FCITX5) << "AddonModel::dataForItem - index:" << index << "role:" << role;
    const auto &addonList = addonEntryList_[index.parent().row()].second;
    const auto &addon = addonList[index.row()];

    switch (role) {

    case Qt::DisplayRole:
        return addon.name();

    case CommentRole:
        return addon.comment();

    case ConfigurableRole:
        return addon.configurable();

    case AddonNameRole:
        return addon.uniqueName();

    case CategoryRole:
        return addon.category();

    case Qt::CheckStateRole:
        if (disabledList_.contains(addon.uniqueName())) {
            return false;
        } else if (enabledList_.contains(addon.uniqueName())) {
            return true;
        }
        return addon.enabled();

    case RowTypeRole:
        return AddonType;
    }
    return QVariant();
}

bool AddonModel::setData(const QModelIndex &index, const QVariant &value,
                         int role) {
    // qCDebug(KCM_FCITX5) << "AddonModel::setData - index:" << index << "value:" << value << "role:" << role;

    if (!index.isValid() || !index.parent().isValid() ||
        index.parent().row() >= addonEntryList_.size() ||
        index.parent().column() > 0 || index.column() > 0) {
        // qCWarning(KCM_FCITX5) << "Invalid index for setData";
        return false;
    }

    const auto &addonList = addonEntryList_[index.parent().row()].second;

    if (index.row() >= addonList.size()) {
        // qCWarning(KCM_FCITX5) << "Row index out of bounds for setData";
        return false;
    }

    bool ret = false;

    auto &item = addonList[index.row()];
    if (role == Qt::CheckStateRole) {
        auto oldData = data(index, role).toBool();
        auto enabled = value.toBool();
        if (item.enabled() == enabled) {
            enabledList_.remove(item.uniqueName());
            disabledList_.remove(item.uniqueName());
        } else if (enabled) {
            enabledList_.insert(item.uniqueName());
            disabledList_.remove(item.uniqueName());
        } else {
            enabledList_.remove(item.uniqueName());
            disabledList_.insert(item.uniqueName());
        }
        auto newData = data(index, role).toBool();
        ret = oldData != newData;

        if (ret) {
            // qCInfo(KCM_FCITX5) << "Addon" << item.uniqueName() << "check state changed to" << newData;
            Q_EMIT dataChanged(index, index);
            Q_EMIT changed(item.uniqueName(), newData);
        }
    }

    qCDebug(KCM_FCITX5) << "Exiting AddonModel::setData, changed:" << ret;
    return ret;
}
QModelIndex AddonModel::findAddon(const QString &addon) const {
    // qCDebug(KCM_FCITX5) << "AddonModel::findAddon - addon:" << addon;
    for (int i = 0; i < addonEntryList_.size(); i++) {
        for (int j = 0; j < addonEntryList_[i].second.size(); j++) {
            const auto &addonList = addonEntryList_[i].second;
            if (addonList[j].uniqueName() == addon) {
                return index(j, 0, index(i, 0));
            }
        }
    }
    // qCDebug(KCM_FCITX5) << "Addon not found";
    return QModelIndex();
}

FlatAddonModel::FlatAddonModel(QObject *parent) : QAbstractListModel(parent) {
    // qCDebug(KCM_FCITX5) << "FlatAddonModel constructor";
}

bool FlatAddonModel::setData(const QModelIndex &index, const QVariant &value,
                             int role) {
    // qCDebug(KCM_FCITX5) << "Entering FlatAddonModel::setData for index" << index.row() << "with role" << role;
    if (!index.isValid() || index.row() >= addonEntryList_.size() ||
        index.column() > 0) {
        // qCWarning(KCM_FCITX5) << "Invalid index for setData";
        return false;
    }

    bool ret = false;

    if (role == Qt::CheckStateRole) {
        // qCDebug(KCM_FCITX5) << "role is Qt::CheckStateRole";
        auto oldData = data(index, role).toBool();
        auto &item = addonEntryList_[index.row()];
        auto enabled = value.toBool();
        if (item.enabled() == enabled) {
            enabledList_.remove(item.uniqueName());
            disabledList_.remove(item.uniqueName());
        } else if (enabled) {
            enabledList_.insert(item.uniqueName());
            disabledList_.remove(item.uniqueName());
        } else {
            enabledList_.remove(item.uniqueName());
            disabledList_.insert(item.uniqueName());
        }
        auto newData = data(index, role).toBool();
        ret = oldData != newData;
    }

    if (ret) {
        // qCDebug(KCM_FCITX5) << "ret is true";
        Q_EMIT dataChanged(index, index);
        Q_EMIT changed();
    }

    // qCDebug(KCM_FCITX5) << "Exiting FlatAddonModel::setData, changed:" << ret;
    return ret;
}

QVariant FlatAddonModel::data(const QModelIndex &index, int role) const {
    // qCDebug(KCM_FCITX5) << "FlatAddonModel::data - index:" << index << "role:" << role;
    if (!index.isValid() || index.row() >= addonEntryList_.size()) {
        // qCDebug(KCM_FCITX5) << "index is invalid";
        return QVariant();
    }

    const auto &addon = addonEntryList_.at(index.row());

    switch (role) {

    case Qt::DisplayRole:
        // qCDebug(KCM_FCITX5) << "role is Qt::DisplayRole";
        return addon.name();

    case CommentRole:
        // qCDebug(KCM_FCITX5) << "role is CommentRole";
        return addon.comment();

    case ConfigurableRole:
        // qCDebug(KCM_FCITX5) << "role is ConfigurableRole";
        return addon.configurable();

    case AddonNameRole:
        // qCDebug(KCM_FCITX5) << "role is AddonNameRole";
        return addon.uniqueName();

    case CategoryRole:
        // qCDebug(KCM_FCITX5) << "role is CategoryRole";
        return addon.category();

    case CategoryNameRole:
        // qCDebug(KCM_FCITX5) << "role is CategoryNameRole";
        return categoryName(addon.category());

    case DependenciesRole:
        // qCDebug(KCM_FCITX5) << "role is DependenciesRole";
        return reverseDependencies_.value(addon.uniqueName());
        ;

    case OptDependenciesRole:
        // qCDebug(KCM_FCITX5) << "role is OptDependenciesRole";
        return reverseOptionalDependencies_.value(addon.uniqueName());

    case Qt::CheckStateRole:
        if (disabledList_.contains(addon.uniqueName())) {
            // qCDebug(KCM_FCITX5) << "role is Qt::CheckStateRole and addon is disabled";
            return false;
        } else if (enabledList_.contains(addon.uniqueName())) {
            // qCDebug(KCM_FCITX5) << "role is Qt::CheckStateRole and addon is enabled";
            return true;
        }
        // qCDebug(KCM_FCITX5) << "role is Qt::CheckStateRole and addon is enabled";
        return addon.enabled();

    case RowTypeRole:
        return AddonType;
    }
    return QVariant();
}

int FlatAddonModel::rowCount(const QModelIndex &parent) const {
    // qCDebug(KCM_FCITX5) << "FlatAddonModel::rowCount - parent:" << parent;
    if (parent.isValid()) {
        // qCDebug(KCM_FCITX5) << "FlatAddonModel::rowCount - parent is valid";
        return 0;
    }

    return addonEntryList_.count();
}

QHash<int, QByteArray> FlatAddonModel::roleNames() const {
    // qCDebug(KCM_FCITX5) << "FlatAddonModel::roleNames";
    return {{Qt::DisplayRole, "name"},
            {CommentRole, "comment"},
            {ConfigurableRole, "configurable"},
            {AddonNameRole, "uniqueName"},
            {CategoryRole, "category"},
            {CategoryNameRole, "categoryName"},
            {Qt::CheckStateRole, "enabled"},
            {DependenciesRole, "dependencies"},
            {OptDependenciesRole, "optionalDependencies"}};
}

void FlatAddonModel::setAddons(const fcitx::FcitxQtAddonInfoV2List &list) {
    // qCDebug(KCM_FCITX5) << "FlatAddonModel::setAddons - list size:" << list.size();
    beginResetModel();
    addonEntryList_ = list;
    nameToAddonMap_.clear();
    reverseDependencies_.clear();
    reverseOptionalDependencies_.clear();
    for (const auto &addon : list) {
        nameToAddonMap_[addon.uniqueName()] = addon;
    }
    for (const auto &addon : list) {
        for (const auto &dep : addon.dependencies()) {
            if (!nameToAddonMap_.contains(dep)) {
                continue;
            }
            reverseDependencies_[dep].append(addon.uniqueName());
        }
        for (const auto &dep : addon.optionalDependencies()) {
            if (!nameToAddonMap_.contains(dep)) {
                continue;
            }
            reverseOptionalDependencies_[dep].append(addon.uniqueName());
        }
    }
    enabledList_.clear();
    disabledList_.clear();
    endResetModel();
    // qCDebug(KCM_FCITX5) << "Exiting FlatAddonModel::setAddons";
}

void FlatAddonModel::enable(const QString &addon) {
    qCDebug(KCM_FCITX5) << "FlatAddonModel::enable - addon:" << addon;
    for (int i = 0; i < addonEntryList_.size(); i++) {
        if (addonEntryList_[i].uniqueName() == addon) {
            qCInfo(KCM_FCITX5) << "Enabling addon" << addon << "at index" << i;
            setData(index(i, 0), true, Qt::CheckStateRole);
            return;
        }
    }
    qCWarning(KCM_FCITX5) << "Could not find addon" << addon << "to enable.";
}

bool AddonProxyModel::filterAcceptsRow(int sourceRow,
                                       const QModelIndex &sourceParent) const {
    Q_UNUSED(sourceParent)
    // qCDebug(KCM_FCITX5) << "AddonProxyModel::filterAcceptsRow - sourceRow:" << sourceRow << "sourceParent:" << sourceParent;

    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    if (index.data(RowTypeRole) == CategoryType) {
        // qCDebug(KCM_FCITX5) << "Filtering category";
        return filterCategory(index);
    }

    // qCDebug(KCM_FCITX5) << "Filtering addon";
    return filterAddon(index);
}

bool AddonProxyModel::filterCategory(const QModelIndex &index) const {
    // qCDebug(KCM_FCITX5) << "Filtering category";
    int childCount = index.model()->rowCount(index);
    if (childCount == 0)
        return false;

    for (int i = 0; i < childCount; ++i) {
        if (filterAddon(index.model()->index(i, 0, index))) {
            // qCDebug(KCM_FCITX5) << "Filtering addon in category";
            return true;
        }
    }
    // qCDebug(KCM_FCITX5) << "Filtering category returning false";
    return false;
}

bool AddonProxyModel::filterAddon(const QModelIndex &index) const {
    // qCDebug(KCM_FCITX5) << "Filtering addon";
    auto name = index.data(Qt::DisplayRole).toString();
    auto uniqueName = index.data(AddonNameRole).toString();
    auto comment = index.data(CommentRole).toString();

    if (!filterText_.isEmpty()) {
        // qCDebug(KCM_FCITX5) << "Filtering addon contains filter text";
        return name.contains(filterText_, Qt::CaseInsensitive) ||
               uniqueName.contains(filterText_, Qt::CaseInsensitive) ||
               comment.contains(filterText_, Qt::CaseInsensitive);
    }

    // qCDebug(KCM_FCITX5) << "Filtering addon returning true";
    return true;
}

bool AddonProxyModel::lessThan(const QModelIndex &left,
                               const QModelIndex &right) const {
    // qCDebug(KCM_FCITX5) << "LessThan - left:" << left << "right:" << right;
    int lhs = left.data(CategoryRole).toInt();
    int rhs = right.data(CategoryRole).toInt();
    // Reorder the addon category.
    // UI and module are more common, because input method config is accessible
    // in the main page.
    static const QMap<int, int> category = {
        {static_cast<int>(AddonCategory::UI), 0},
        {static_cast<int>(AddonCategory::Module), 1},
        {static_cast<int>(AddonCategory::InputMethod), 2},
        {static_cast<int>(AddonCategory::Frontend), 3},
        {static_cast<int>(AddonCategory::Loader), 4},
    };

    int lvalue = category.value(lhs, category.size());
    int rvalue = category.value(rhs, category.size());
    int result = lvalue - rvalue;

    if (result < 0) {
        // qCDebug(KCM_FCITX5) << "LessThan - result < 0";
        return true;
    } else if (result > 0) {
        // qCDebug(KCM_FCITX5) << "LessThan - result > 0";
        return false;
    }

    QString l = left.data(Qt::DisplayRole).toString();
    QString r = right.data(Qt::DisplayRole).toString();
    // qCDebug(KCM_FCITX5) << "LessThan - l:" << l << "r:" << r;
    return QCollator().compare(l, r) < 0;
}

void AddonProxyModel::setFilterText(const QString &text) {
    // qCDebug(KCM_FCITX5) << "AddonProxyModel::setFilterText - text:" << text;
    if (filterText_ != text) {
        // qCDebug(KCM_FCITX5) << "Filter text changed to:" << text;
        filterText_ = text;
        invalidate();
    }
    // qCDebug(KCM_FCITX5) << "Exiting AddonProxyModel::setFilterText";
}

} // namespace kcm
} // namespace fcitx
