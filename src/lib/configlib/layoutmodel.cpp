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
    auto idx = index(row, 0);
    if (idx.isValid()) {
        return idx.data(Qt::UserRole).toString();
    }
    return QString();
}

void LanguageModel::append(const QString &name, const QString &language) {
    qCDebug(KCM_FCITX5) << "Adding language:" << name << "code:" << language;
    QStandardItem *item = new QStandardItem(name);
    item->setData(language, Qt::UserRole);
    appendRow(item);
}

void LanguageFilterModel::setLanguage(const QString &language) {
    if (language_ != language) {
        qCDebug(KCM_FCITX5) << "Setting filter language to:" << language;
        language_ = language;
        invalidateFilter();
    }
}

QVariant LanguageFilterModel::layoutInfo(int row) const {
    auto idx = index(row, 0);
    if (idx.isValid()) {
        return idx.data(LayoutInfoRole);
    }
    return QVariant();
}

bool LanguageFilterModel::filterAcceptsRow(int source_row,
                                           const QModelIndex &) const {
    if (language_.isEmpty()) {
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
    return data(left, Qt::DisplayRole).toString() <
           data(right, Qt::DisplayRole).toString();
}

QHash<int, QByteArray> LayoutInfoModel::roleNames() const {
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
    if (!index.isValid() || index.row() >= layoutInfo_.size()) {
        return QVariant();
    }
    const auto &layout = layoutInfo_.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return layout.description();
    case Qt::UserRole:
        return layout.layout();
    case LayoutLanguageRole: {
        QStringList languages;
        languages = layout.languages();
        for (const auto &variants : layout.variants()) {
            languages << variants.languages();
        }
        return languages;
    }
    case LayoutInfoRole:
        return QVariant::fromValue(layout);
    }
    return QVariant();
}

int LayoutInfoModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return layoutInfo_.size();
}

QHash<int, QByteArray> VariantInfoModel::roleNames() const {
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
    if (!index.isValid() || index.row() >= variantInfo_.size()) {
        return QVariant();
    }
    const auto &layout = variantInfo_.at(index.row());

    switch (role) {

    case Qt::DisplayRole:
        return layout.description();

    case Qt::UserRole:
        return layout.variant();

    case LayoutLanguageRole:
        return layout.languages();

    default:
        return QVariant();
    }
}

int VariantInfoModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return variantInfo_.size();
}

} // namespace kcm
} // namespace fcitx
