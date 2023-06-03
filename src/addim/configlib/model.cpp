// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model.h"

#include "glo.h"
#include "xkbrules.h"

#include <fcitx-utils/i18n.h>

#include <DPinyin>
#include <DStyleOption>

#include <QCollator>
#include <QDebug>
#include <QLocale>
#include <QSize>
#include <QColor>

DCORE_USE_NAMESPACE

CategorizedItemModel::CategorizedItemModel(QObject *parent)
    : QAbstractItemModel(parent) {}

int CategorizedItemModel::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return listSize();
    }

    if (parent.internalId() > 0) {
        return 0;
    }

    if (parent.column() > 0 || parent.row() >= listSize()) {
        return 0;
    }

    return subListSize(parent.row());
}

int CategorizedItemModel::columnCount(const QModelIndex &) const { return 1; }

QModelIndex CategorizedItemModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) {
        return QModelIndex();
    }

    int row = child.internalId();
    if (row && row - 1 >= listSize()) {
        return QModelIndex();
    }

    return createIndex(row - 1, 0, -1);
}

QModelIndex CategorizedItemModel::index(int row, int column,
                                        const QModelIndex &parent) const {
    // return language index
    if (!parent.isValid()) {
        if (column > 0 || row >= listSize()) {
            return QModelIndex();
        } else {
            return createIndex(row, column, static_cast<quintptr>(0));
        }
    }

    // return im index
    if (parent.column() > 0 || parent.row() >= listSize() ||
        row >= subListSize(parent.row())) {
        return QModelIndex();
    }

    return createIndex(row, column, parent.row() + 1);
}

QVariant CategorizedItemModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (!index.parent().isValid()) {
        if (index.column() > 0 || index.row() >= listSize()) {
            return QVariant();
        }

        return dataForCategory(index, role);
    }

    if (index.column() > 0 || index.parent().column() > 0 ||
        index.parent().row() >= listSize()) {
        return QVariant();
    }

    if (index.row() >= subListSize(index.parent().row())) {
        return QVariant();
    }
    return dataForItem(index, role);
}

static QString languageName(const QString &uniqueName, const QString &langCode) {
    const auto &xkbrules = XkbRules::instance();
    QString langName;

    if (uniqueName.startsWith("keyboard-")) {
        auto layout = xkbrules.layout(uniqueName);
        langName = layout.description;
    }

    if (!langName.isEmpty()) {
        return XkbRules::tr(langName);
    }

    if (langCode.isEmpty()) {
        return _("Unknown");
    } else if (langCode == "*") {
        return _("Multilingual");
    }

    langName = QLocale(langCode).nativeLanguageName();
    if (langName.isEmpty()) {
        langName = "Unknown";
        qInfo("NOTICE: uniqueName [%s] not found english name. unknown.", uniqueName.toStdString().c_str());
    }

    return langName;
}

static QString getEnglishLanguageName(const QString &uniqueName, const QString &langCode)
{
    const auto &xkbrules = XkbRules::instance();
    QString englishName;

    if (uniqueName.startsWith("keyboard-")) {
        auto layout = xkbrules.layout(uniqueName);
        englishName = layout.description;
    }

    if (!englishName.isEmpty()) {
        return englishName;
    }

    if (langCode.isEmpty()) {
        return "Unknown";
    } else if (langCode == "*") {
        return "Multilingual";
    }

    englishName = QLocale::languageToString(QLocale(langCode).language());
    if (englishName.isEmpty()) {
        englishName = "Unknown";
        qInfo("NOTICE: uniqueName [%s] not found name. unknown.", uniqueName.toStdString().c_str());
    }

    return englishName;
}

AvailIMModel::AvailIMModel(QObject *parent) : CategorizedItemModel(parent) {}

QVariant AvailIMModel::dataForCategory(const QModelIndex &index,
                                       int role) const {
    switch (role) {

    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        return m_filteredIMEntryList[index.row()].first;

    case FcitxLanguageRole:
        return m_filteredIMEntryList[index.row()].second.at(0).languageCode();

    case FcitxIMUniqueNameRole:
        return QString();

    case FcitxRowTypeRole:
        return LanguageType;

    // 设置背景色
    case Dtk::ViewItemBackgroundRole:
        return QVariant::fromValue(QPair<int, int>{QPalette::Base, DPalette::NoType});

    default:
        return QVariant();
    }
}

QVariant AvailIMModel::dataForItem(const QModelIndex &index, int role) const {
    const FcitxQtInputMethodEntryList &imEntryList =
        m_filteredIMEntryList[index.parent().row()].second;

    const FcitxQtInputMethodEntry &imEntry = imEntryList[index.row()];

    switch (role) {

    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        return imEntry.name();

    case FcitxRowTypeRole:
        return IMType;

    case FcitxIMUniqueNameRole:
        return imEntry.uniqueName();

    case FcitxLanguageRole:
        return imEntry.languageCode();

    // 设置背景色
    case Dtk::ViewItemBackgroundRole:
        return QVariant::fromValue(QPair<int, int>{QPalette::Base, DPalette::NoType});

    }
    return QVariant();
}

static QString buildKeyByIMNameAndLanguageCode(const QString &uniqueName, const QString &languageCode) {
    auto translatedName = languageName(uniqueName, languageCode);
    auto englishName = getEnglishLanguageName(uniqueName, languageCode);
    QString key = translatedName + " - " + englishName;

    return key;
}

void AvailIMModel::filterIMEntryList(
    const FcitxQtInputMethodEntryList &imEntryList,
    const FcitxQtStringKeyValueList &enabledIMList) {

    beginResetModel();

    QMap<QString, int> languageMap;
    m_filteredIMEntryList.clear();

    QSet<QString> enabledIMs;
    for (const auto &item : enabledIMList) {
        enabledIMs.insert(item.key());
    }

    for (const FcitxQtInputMethodEntry &im : imEntryList) {
        if (enabledIMs.contains(im.uniqueName())) {
            continue;
        }
        QString key = buildKeyByIMNameAndLanguageCode(im.uniqueName(), im.languageCode());
        int idx;
        if (!languageMap.contains(key)) {
            idx = m_filteredIMEntryList.count();
            languageMap[key] = idx;
            m_filteredIMEntryList.append(
                QPair<QString, FcitxQtInputMethodEntryList>(
                    key, FcitxQtInputMethodEntryList()));
        } else {
            idx = languageMap[key];
        }
        m_filteredIMEntryList[idx].second.append(im);
    }
    endResetModel();
}

IMProxyModel::IMProxyModel(QObject *parent) : QSortFilterProxyModel(parent) {
    setDynamicSortFilter(true);
    sort(0);
}

void IMProxyModel::setFilterText(const QString &text) {
    if (m_filterText != text) {
        m_filterText = text;
        invalidate();
    }
}

void IMProxyModel::setShowOnlyCurrentLanguage(bool show) {
    if (m_showOnlyCurrentLanguage != show) {
        m_showOnlyCurrentLanguage = show;
        invalidate();
    }
}

void IMProxyModel::filterIMEntryList(
    const FcitxQtInputMethodEntryList &imEntryList,
    const FcitxQtStringKeyValueList &enabledIMList) {
    m_languageSet.clear();

    QSet<QString> enabledIMs;
    for (const auto &item : enabledIMList) {
        enabledIMs.insert(item.key());
    }
    for (const FcitxQtInputMethodEntry &im : imEntryList) {
        if (enabledIMs.contains(im.uniqueName())) {
            m_languageSet.insert(im.languageCode().left(2));
        }
    }
    invalidate();
}

bool IMProxyModel::filterAcceptsRow(int source_row,
                                    const QModelIndex &source_parent) const {
    const QModelIndex index =
        sourceModel()->index(source_row, 0, source_parent);

    if (index.data(FcitxRowTypeRole) == LanguageType) {
        return filterLanguage(index);
    }

    return filterIM(index);
}

bool IMProxyModel::filterLanguage(const QModelIndex &index) const {
    if (!index.isValid()) {
        return false;
    }

    int childCount = index.model()->rowCount(index);
    if (childCount == 0)
        return false;

    for (int i = 0; i < childCount; ++i) {
        if (filterIM(index.model()->index(i, 0, index))) {
            return true;
        }
    }

    return false;
}

bool IMProxyModel::filterIM(const QModelIndex &index) const {
    QString uniqueName = index.data(FcitxIMUniqueNameRole).toString();
    QString name = index.data(Qt::DisplayRole).toString();
    QString langCode = index.data(FcitxLanguageRole).toString();

    // Always show keyboard us if we are not searching.
    if (uniqueName == "keyboard-us" && m_filterText.isEmpty()) {
        return true;
    }

    bool flag = true;
    QString lang = langCode.left(2);
    bool showOnlyCurrentLanguage =
        m_filterText.isEmpty() && m_showOnlyCurrentLanguage;

    flag =
        flag && (showOnlyCurrentLanguage
                     ? !lang.isEmpty() && (QLocale().name().startsWith(lang) ||
                                           m_languageSet.contains(lang))
                     : true);
    if (!m_filterText.isEmpty()) {
        flag = flag && (name.contains(m_filterText, Qt::CaseInsensitive) ||
                        uniqueName.contains(m_filterText, Qt::CaseInsensitive) ||
                        langCode.contains(m_filterText, Qt::CaseInsensitive) ||
                        languageName(uniqueName, langCode).contains(m_filterText,
                                                        Qt::CaseInsensitive));
    }
    return flag;
}

bool IMProxyModel::lessThan(const QModelIndex &left,
                            const QModelIndex &right) const {
    int result = compareCategories(left, right);
    if (result < 0) {
        return true;
    } else if (result > 0) {
        return false;
    }

    QString l = left.data(Qt::DisplayRole).toString();
    QString r = right.data(Qt::DisplayRole).toString();
    return QCollator().compare(l, r) < 0;
}

int IMProxyModel::compareCategories(const QModelIndex &left,
                                    const QModelIndex &right) const {
    int leftH = 0;
    auto leftChildrenCount = sourceModel()->rowCount(left);
    if (leftChildrenCount != 0) {
        for (int r = 0; r < leftChildrenCount; r++) {
            auto i = sourceModel()->index(r, 0, left);
            QString lang = i.data(FcitxLanguageRole).toString();
            if (lang.isEmpty()) {
                continue;
            }

            if (QLocale().name() == lang) {
                leftH = 1;
                break;
            }

            if (QLocale().name().startsWith(lang.left(2))) {
                leftH = 2;
            }
        }
    }

    int rightH = 0;
    auto rightChildrenCount = sourceModel()->rowCount(right);
    if (rightChildrenCount != 0) {
        for (int r = 0; r < rightChildrenCount; r++) {
            auto i = sourceModel()->index(r, 0, right);
            QString lang = i.data(FcitxLanguageRole).toString();
            if (lang.isEmpty()) {
                continue;
            }

            if (QLocale().name() == lang) {
                rightH = 1;
                break;
            }

            if (QLocale().name().startsWith(lang.left(2))) {
                rightH = 2;
            }
        }
    }

    if (leftH == rightH) {
        return 0;
    }

    if (leftH == 1) {
        return -1;
    }

    if (rightH == 1) {
        return 1;
    }

    if (leftH == 2) {
        return -1;
    }

    if (rightH == 2) {
        return 1;
    }

    return 0;
}

FilteredIMModel::FilteredIMModel(Mode mode, QObject *parent)
    : QAbstractListModel(parent), m_mode(mode) {}

QVariant FilteredIMModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_filteredIMEntryList.size()) {
        return QVariant();
    }

    const FcitxQtInputMethodEntry &imEntry =
        m_filteredIMEntryList.at(index.row());

    switch (role) {

    case Qt::DisplayRole:
        return imEntry.name();

    case FcitxRowTypeRole:
        return IMType;

    case FcitxIMUniqueNameRole:
        return imEntry.uniqueName();

    case FcitxLanguageRole:
        return imEntry.languageCode();

    case FcitxIMConfigurableRole:
        return imEntry.configurable();

    case FcitxLanguageNameRole:
        return languageName(imEntry.uniqueName(), imEntry.languageCode());

    case FcitxIMLayoutRole: {
        auto iter = std::find_if(m_enabledIMList.begin(), m_enabledIMList.end(),
                                 [&imEntry](const FcitxQtStringKeyValue &item) {
                                     return item.key() == imEntry.uniqueName();
                                 });
        if (iter != m_enabledIMList.end()) {
            return iter->value();
        }
        return QString();
    }
    case FcitxIMActiveRole:
        return index.row() > 0 ? QString("active") : QString("inactive");

    default:
        return QVariant();
    }
}

int FilteredIMModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return m_filteredIMEntryList.count();
}

QHash<int, QByteArray> FilteredIMModel::roleNames() const {
    return {{Qt::DisplayRole, "name"},
            {FcitxIMUniqueNameRole, "uniqueName"},
            {FcitxLanguageRole, "languageCode"},
            {FcitxLanguageNameRole, "language"},
            {FcitxIMConfigurableRole, "configurable"},
            {FcitxIMLayoutRole, "layout"},
            {FcitxIMActiveRole, "active"}};
}

void FilteredIMModel::filterIMEntryList(
    const FcitxQtInputMethodEntryList &imEntryList,
    const FcitxQtStringKeyValueList &enabledIMList) {
    beginResetModel();

    m_filteredIMEntryList.clear();
    m_enabledIMList = enabledIMList;

    // We implement this twice for following reasons:
    // 1. "enabledIMs" is usually very small.
    // 2. CurrentIM mode need to keep order by enabledIMs.
    if (m_mode == CurrentIM) {
        int row = 0;
        QMap<QString, const FcitxQtInputMethodEntry *> nameMap;
        for (auto &imEntry : imEntryList) {
            nameMap.insert(imEntry.uniqueName(), &imEntry);
        }

        for (const auto &im : enabledIMList) {
            if (auto value = nameMap.value(im.key(), nullptr)) {
                m_filteredIMEntryList.append(*value);
                row++;
            }
        }
    } else if (m_mode == AvailIM) {
        QSet<QString> enabledIMs;
        for (const auto &item : enabledIMList) {
            enabledIMs.insert(item.key());
        }

        for (const FcitxQtInputMethodEntry &im : imEntryList) {
            if (enabledIMs.contains(im.uniqueName())) {
                continue;
            }
            m_filteredIMEntryList.append(im);
        }
    }
    endResetModel();
}

void FilteredIMModel::move(int from, int to) {
    if (from < 0 || from >= m_filteredIMEntryList.size() || to < 0 ||
        to >= m_filteredIMEntryList.size()) {
        return;
    }
    beginMoveRows(QModelIndex(), from, from, QModelIndex(),
                  to > from ? to + 1 : to);
    m_filteredIMEntryList.move(from, to);
    endMoveRows();
    Q_EMIT imListChanged(m_filteredIMEntryList);
}

void FilteredIMModel::remove(int idx) {
    if (idx < 0 || idx >= m_filteredIMEntryList.size()) {
        return;
    }
    beginRemoveRows(QModelIndex(), idx, idx);
    m_filteredIMEntryList.removeAt(idx);
    endRemoveRows();
    Q_EMIT imListChanged(m_filteredIMEntryList);
}
