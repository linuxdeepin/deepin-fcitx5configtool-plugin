// SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#include "layoutmodel.h"

namespace fcitx {
namespace addim {

LanguageModel::LanguageModel(QObject *parent) : QStandardItemModel(parent) {
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
    QStandardItem *item = new QStandardItem(name);
    item->setData(language, Qt::UserRole);
    appendRow(item);
}

void LanguageFilterModel::setLanguage(const QString &language) {
    if (m_language != language) {
        m_language = language;
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
    if (m_language.isEmpty()) {
        return true;
    }

    auto index = sourceModel()->index(source_row, 0);
    return sourceModel()
        ->data(index, LayoutLanguageRole)
        .toStringList()
        .contains(m_language);
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
    beginResetModel();
    m_layoutInfo = std::move(info);
    endResetModel();
}

QVariant LayoutInfoModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_layoutInfo.size()) {
        return QVariant();
    }
    const auto &layout = m_layoutInfo.at(index.row());

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

    return m_layoutInfo.size();
}

QHash<int, QByteArray> VariantInfoModel::roleNames() const {
    return {
        {Qt::DisplayRole, "name"},
        {Qt::UserRole, "variant"},
        {LayoutLanguageRole, "language"},
    };
}

void VariantInfoModel::setVariantInfo(const FcitxQtLayoutInfo &info) {
    beginResetModel();
    m_variantInfo.clear();
    FcitxQtVariantInfo defaultVariant;
    defaultVariant.setVariant("");
    defaultVariant.setDescription(_("Default"));
    defaultVariant.setLanguages(info.languages());
    m_variantInfo << defaultVariant;
    m_variantInfo << info.variants();
    endResetModel();
}

QVariant VariantInfoModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_variantInfo.size()) {
        return QVariant();
    }
    const auto &layout = m_variantInfo.at(index.row());

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

    return m_variantInfo.size();
}

} // namespace addim
} // namespace fcitx
