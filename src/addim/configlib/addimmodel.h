/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef ADDIMMODEL_H
#define ADDIMMODEL_H

#include <QAbstractListModel>
#include <QSet>
#include <QSortFilterProxyModel>
#include <fcitxqtdbustypes.h>

using namespace fcitx;

enum {
    FcitxRowTypeRole = 0x324da8fc,
    FcitxLanguageRole,
    FcitxLanguageNameRole,
    FcitxIMUniqueNameRole,
    FcitxIMConfigurableRole,
    FcitxIMLayoutRole,

    FcitxLanguageCode,     // 取得语言编码
    FcitxUseIMLanguageRole,// 是否为使用的输入法语言项（有一个输入法子项被使用，语言项则被使用）
    FcitxRowIndexRole,
    FcitxUseIMRole,        // 是否为使用的输入法项
    FcitxEnglishNameRole,  // 取得语言的英文名称
};

enum { LanguageType, IMType };

class IMConfigModelInterface {
public:
    virtual ~IMConfigModelInterface() = default;
    virtual void
    filterIMEntryList(const FcitxQtInputMethodEntryList &imEntryList,
                      const FcitxQtStringKeyValueList &enabledIMs) = 0;
};

class CategorizedItemModel : public QAbstractItemModel {
    Q_OBJECT
public:
    CategorizedItemModel(QObject *parent = 0);
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

protected:
    virtual int listSize() const = 0;
    virtual int subListSize(int idx) const = 0;
    virtual QVariant dataForItem(const QModelIndex &index, int role) const = 0;
    virtual QVariant dataForCategory(const QModelIndex &index,
                                     int role) const = 0;
};

class AvailIMModel : public CategorizedItemModel,
                     public IMConfigModelInterface {
    Q_OBJECT
public:
    AvailIMModel(QObject *parent = 0);
    void
    filterIMEntryList(const FcitxQtInputMethodEntryList &imEntryList,
                      const FcitxQtStringKeyValueList &enabledIMs) override;

    void getInputMethodEntryList(int row, FcitxQtStringKeyValueList& currentNameList);
    void getInputMethodEntryList(int row, FcitxQtStringKeyValueList& currentNameList, FcitxQtStringKeyValueList& currentUseNameList, FcitxQtStringKeyValueList& useIMList);
    void getInputMethodEntryList(int row, FcitxQtStringKeyValueList& currentNameList, FcitxQtStringKeyValueList& currentUseNameList, FcitxQtStringKeyValueList& useIMList, QString matchStr);
    bool existUseIMEntryList(int row, FcitxQtStringKeyValueList& useIMList) const;

protected:
    int listSize() const override { return m_filteredIMEntryList.size(); }
    int subListSize(int idx) const override {
        return m_filteredIMEntryList[idx].second.size();
    }
    QVariant dataForItem(const QModelIndex &index, int role) const override;
    QVariant dataForCategory(const QModelIndex &index, int role) const override;

private:
    QList<QPair<QString, bool>> m_filteredUseIMLanguageList;
    QList<QPair<QString, FcitxQtInputMethodEntryList>> m_filteredIMEntryList;
};

class IMProxyModel : public QSortFilterProxyModel,
                     public IMConfigModelInterface {
    Q_OBJECT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText);
    Q_PROPERTY(bool showOnlyCurrentLanguage READ showOnlyCurrentLanguage WRITE
                   setShowOnlyCurrentLanguage);

public:
    IMProxyModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override {
        if (sourceModel()) {
            return sourceModel()->roleNames();
        }
        return QSortFilterProxyModel::roleNames();
    }

    const QString &filterText() const { return m_filterText; }
    void setFilterText(const QString &text);
    bool showOnlyCurrentLanguage() const { return m_showOnlyCurrentLanguage; }
    void setShowOnlyCurrentLanguage(bool checked);

    void
    filterIMEntryList(const FcitxQtInputMethodEntryList &imEntryList,
                      const FcitxQtStringKeyValueList &enabledIMs) override;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    int compareCategories(const QModelIndex &left, const QModelIndex &right) const;

private:
    bool filterLanguage(const QModelIndex &index) const;
    bool filterIM(const QModelIndex &index) const;

    bool m_showOnlyCurrentLanguage = false;
    QString m_filterText;
    QSet<QString> m_languageSet;
};

class FilteredIMModel : public QAbstractListModel,
                        public IMConfigModelInterface {
    Q_OBJECT
    Q_PROPERTY(int count READ count);

public:
    enum Mode { CurrentIM, AvailIM };

    FilteredIMModel(Mode mode, QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;
    void filterIMEntryList(const FcitxQtInputMethodEntryList &imEntryList, const FcitxQtStringKeyValueList &enabledIMs) override;

    int count() const { return rowCount(); }
    Q_INVOKABLE QString imAt(int idx) const {
        return index(idx).data(FcitxIMUniqueNameRole).toString();
    }
public slots:
    void move(int from, int to);
    void remove(int index);

signals:
    void imListChanged(FcitxQtInputMethodEntryList list);

private:
    Mode m_mode;
    FcitxQtInputMethodEntryList m_filteredIMEntryList;
    FcitxQtStringKeyValueList m_enabledIMList;
};

#endif
