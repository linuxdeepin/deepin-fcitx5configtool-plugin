// SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _CONFIGLIB_LAYOUTMODEL_H_
#define _CONFIGLIB_LAYOUTMODEL_H_

#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <fcitx-utils/i18n.h>
#include <fcitxqtdbustypes.h>

namespace fcitx {
namespace addim {

enum { LayoutLanguageRole = 0x3423545, LayoutInfoRole };

class LanguageModel : public QStandardItemModel {
    Q_OBJECT
public:
    LanguageModel(QObject *parent = nullptr);
    Q_INVOKABLE QString language(int row) const;
    void append(const QString &name, const QString &language);
};

class LanguageFilterModel : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(QString language READ language WRITE setLanguage);

public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
    // Forward role names.
    QHash<int, QByteArray> roleNames() const override {
        if (sourceModel()) {
            return sourceModel()->roleNames();
        }
        return QSortFilterProxyModel::roleNames();
    }

    const QString &language() const { return m_language; }
    void setLanguage(const QString &language);

    Q_INVOKABLE QVariant layoutInfo(int row) const;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &) const override;
    bool lessThan(const QModelIndex &left,
                  const QModelIndex &right) const override;

private:
    QString m_language;
};

class LayoutInfoModel : public QAbstractListModel {
    Q_OBJECT
public:
    using QAbstractListModel::QAbstractListModel;
    QHash<int, QByteArray> roleNames() const override;

    auto &layoutInfo() const { return m_layoutInfo; }
    void setLayoutInfo(FcitxQtLayoutInfoList info);

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    FcitxQtLayoutInfoList m_layoutInfo;
};

class VariantInfoModel : public QAbstractListModel {
    Q_OBJECT
public:
    using QAbstractListModel::QAbstractListModel;
    QHash<int, QByteArray> roleNames() const override;

    auto &variantInfo() const { return m_variantInfo; }
    void setVariantInfo(const FcitxQtLayoutInfo &info);

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    FcitxQtVariantInfoList m_variantInfo;
};

} // namespace addim
} // namespace fcitx

#endif // _CONFIGLIB_LAYOUTMODEL_H_
