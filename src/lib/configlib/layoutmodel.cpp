/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "layoutmodel.h"
#include "logging.h"

namespace fcitx {
namespace kcm {

LanguageModel::LanguageModel(QObject *parent) : QStandardItemModel(parent) {
    qCDebug(KCM_FCITX5) << "Initializing LanguageModel";
    setItemRoleNames({{Qt::DisplayRole, "name"}, {Qt::UserRole, "language"}});
}

QString LanguageModel::language(int row) const {
    qCDebug(KCM_FCITX5) << "Entering LanguageModel::language for row" << row;
    auto idx = index(row, 0);
    if (idx.isValid()) {
        return idx.data(Qt::UserRole).toString();
    }
    qCWarning(KCM_FCITX5) << "Invalid index for row" << row;
    return QString();
}

void LanguageModel::append(const QString &name, const QString &language) {
    qCDebug(KCM_FCITX5) << "Adding language:" << name << "code:" << language;
    QStandardItem *item = new QStandardItem(name);
    item->setData(language, Qt::UserRole);
    appendRow(item);
}

void LanguageFilterModel::setLanguage(const QString &language) {
    qCDebug(KCM_FCITX5) << "Setting filter language to:" << language;
    if (language_ != language) {
        qCDebug(KCM_FCITX5) << "Setting filter language to:" << language;
        language_ = language;
        invalidateFilter();
    }
}

QVariant LanguageFilterModel::layoutInfo(int row) const {
    qCDebug(KCM_FCITX5) << "Entering LanguageFilterModel::layoutInfo for row" << row;
    auto idx = index(row, 0);
    if (idx.isValid()) {
        return idx.data(LayoutInfoRole);
    }
    qCWarning(KCM_FCITX5) << "Invalid index for row" << row;
    return QVariant();
}

bool LanguageFilterModel::filterAcceptsRow(int source_row,
                                           const QModelIndex &) const {
    // qCDebug(KCM_FCITX5) << "Filtering row" << source_row << "for language:" << language_;
    if (language_.isEmpty()) {
        // qCDebug(KCM_FCITX5) << "Language is empty, accepting all rows";
        return true;
    }

    auto index = sourceModel()->index(source_row, 0);
    return sourceModel()
        ->data(index, LayoutLanguageRole)
        .toStringList()
        .contains(language_);
}
bool LanguageFilterModel::lessThan(const QModelIndex &left,
                                   const QModelIndex &right) const {
    // qCDebug(KCM_FCITX5) << "Comparing rows" << left.row() << "and" << right.row();
    return data(left, Qt::DisplayRole).toString() <
           data(right, Qt::DisplayRole).toString();
}

QHash<int, QByteArray> LayoutInfoModel::roleNames() const {
    // qCDebug(KCM_FCITX5) << "Setting role names";
    return {
        {Qt::DisplayRole, "name"},
        {Qt::UserRole, "layout"},
        {LayoutLanguageRole, "language"},
    };
}

void LayoutInfoModel::setLayoutInfo(FcitxQtLayoutInfoList info) {
    qCDebug(KCM_FCITX5) << "Setting layout info with" << info.size() << "layouts";
    beginResetModel();
    layoutInfo_ = std::move(info);
    endResetModel();
}

QVariant LayoutInfoModel::data(const QModelIndex &index, int role) const {
    // qCDebug(KCM_FCITX5) << "Getting data for index" << index << "with role" << role;
    if (!index.isValid() || index.row() >= layoutInfo_.size()) {
        // qCDebug(KCM_FCITX5) << "Invalid index or row out of bounds";
        return QVariant();
    }
    const auto &layout = layoutInfo_.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        // qCDebug(KCM_FCITX5) << "Display role for index" << index;
        return layout.description();
    case Qt::UserRole:
        // qCDebug(KCM_FCITX5) << "User role for index" << index;
        return layout.layout();
    case LayoutLanguageRole: {
        // qCDebug(KCM_FCITX5) << "Layout language role for index" << index;
        QStringList languages;
        languages = layout.languages();
        for (const auto &variants : layout.variants()) {
            languages << variants.languages();
        }
        return languages;
    }
    case LayoutInfoRole:
        // qCDebug(KCM_FCITX5) << "Layout info role for index" << index;
        return QVariant::fromValue(layout);
    }
    // qCDebug(KCM_FCITX5) << "Returning empty variant for index" << index << "with role" << role;
    return QVariant();
}

int LayoutInfoModel::rowCount(const QModelIndex &parent) const {
    // qCDebug(KCM_FCITX5) << "Returning row count for parent" << parent;
    if (parent.isValid()) {
        // qCDebug(KCM_FCITX5) << "Parent is valid, returning 0";
        return 0;
    }

    // qCDebug(KCM_FCITX5) << "Returning" << layoutInfo_.size() << "rows";
    return layoutInfo_.size();
}

QHash<int, QByteArray> VariantInfoModel::roleNames() const {
    // qCDebug(KCM_FCITX5) << "Setting role names";
    return {
        {Qt::DisplayRole, "name"},
        {Qt::UserRole, "variant"},
        {LayoutLanguageRole, "language"},
    };
}

void VariantInfoModel::setVariantInfo(const FcitxQtLayoutInfo &info) {
    qCDebug(KCM_FCITX5) << "Setting variant info for layout:" << info.layout();
    beginResetModel();
    variantInfo_.clear();
    FcitxQtVariantInfo defaultVariant;
    defaultVariant.setVariant("");
    defaultVariant.setDescription(_("Default"));
    defaultVariant.setLanguages(info.languages());
    variantInfo_ << defaultVariant;
    variantInfo_ << info.variants();
    qCDebug(KCM_FCITX5) << "Loaded" << variantInfo_.size() << "variants";
    endResetModel();
}

QVariant VariantInfoModel::data(const QModelIndex &index, int role) const {
    // qCDebug(KCM_FCITX5) << "Getting data for index" << index << "with role" << role;
    if (!index.isValid() || index.row() >= variantInfo_.size()) {
        // qCDebug(KCM_FCITX5) << "Invalid index or row out of bounds";
        return QVariant();
    }
    const auto &layout = variantInfo_.at(index.row());

    switch (role) {

    case Qt::DisplayRole:
        // qCDebug(KCM_FCITX5) << "Display role for index" << index;
        return layout.description();

    case Qt::UserRole:
        // qCDebug(KCM_FCITX5) << "User role for index" << index;
        return layout.variant();

    case LayoutLanguageRole:
        // qCDebug(KCM_FCITX5) << "Layout language role for index" << index;
        return layout.languages();

    default:
        // qCDebug(KCM_FCITX5) << "Returning empty variant for index" << index << "with role" << role;
        return QVariant();
    }
}

int VariantInfoModel::rowCount(const QModelIndex &parent) const {
    // qCDebug(KCM_FCITX5) << "Returning row count for parent" << parent;
    if (parent.isValid()) {
        // qCDebug(KCM_FCITX5) << "Parent is valid, returning 0";
        return 0;
    }

    // qCDebug(KCM_FCITX5) << "Returning" << variantInfo_.size() << "rows";
    return variantInfo_.size();
}

} // namespace kcm
} // namespace fcitx
