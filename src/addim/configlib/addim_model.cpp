/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "glo.h"
#include "imelog.h"
#include "addim_model.h"
#include <QCollator>
#include <QLocale>
#include <QSize>
#include <fcitx-utils/i18n.h>

#include <DPinyin>
DCORE_USE_NAMESPACE

CategorizedItemModel::CategorizedItemModel(QObject *parent)
    : QAbstractItemModel(parent) {
}

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

QModelIndex CategorizedItemModel::index(int row, int column, const QModelIndex &parent) const {
    if (!parent.isValid()) {
        if (column > 0 || row >= listSize()) {
            return QModelIndex();
        } else {
            return createIndex(row, column, static_cast<quintptr>(0));
        }
    }

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

static QString languageName(const QString &langCode) {
    if (langCode.isEmpty()) {
        return _("Unknown");
    }
    else if (langCode == "*") {
        return _("Multilingual");
    }
    else {
        QLocale locale(langCode);
        if (locale.language() == QLocale::C) {
            // return lang code seems to be a better solution other than
            // indistinguishable "unknown"
            return langCode;
        }
        const bool hasCountry = langCode.indexOf("_") != -1 &&
                                locale.country() != QLocale::AnyCountry;
        QString languageName;
        if (hasCountry) {
            languageName = locale.nativeLanguageName();
        }
        if (languageName.isEmpty()) {
            languageName = "";
            languageName += QLocale::languageToString(locale.language()).toUtf8();
        }
        if (languageName.isEmpty()) {
            languageName = _("Other");
        }
        QString countryName;
        // QLocale will always assign a default country for us, check if our
        // lang code

        if (langCode.indexOf("_") != -1 &&
            locale.country() != QLocale::AnyCountry) {
            countryName = locale.nativeCountryName();
            if (countryName.isEmpty()) {
                countryName = QLocale::countryToString(locale.country());
            }
        }

        if (countryName.isEmpty()) {
            return languageName;
        } else {
            return QString(
                       C_("%1 is language name, %2 is country name", "%1 (%2)"))
                .arg(languageName, countryName);
        }
    }
}

AvailIMModel::AvailIMModel(QObject *parent) : CategorizedItemModel(parent) {}

QVariant AvailIMModel::dataForCategory(const QModelIndex &index, int role) const {
    QString language             = "";
    QString categoryLanguageName = "";
    switch (role) {
    case Qt::DisplayRole:
        language = languageName(m_filteredIMEntryList[index.row()].first);

        categoryLanguageName = buildCategroyLanguageName(language);
        return categoryLanguageName;

    case FcitxLanguageRole:
        return languageName(m_filteredIMEntryList[index.row()].first);

    case FcitxLanguageCode:
        return m_filteredIMEntryList[index.row()].second.at(0).languageCode();

    case FcitxIMUniqueNameRole:
        return QString();

    case FcitxRowTypeRole:
        return LanguageType;

    case FcitxRowIndexRole:
        return index.row();

    case FcitxUseIMLanguageRole: {
        FcitxQtStringKeyValueList useIMList_ = getUseIMList();
        bool exist = existUseIMEntryList(index.row(), useIMList_);
        return exist;
    }

    default:
        return QVariant();
    }
}

QVariant AvailIMModel::dataForItem(const QModelIndex &index, int role) const {
    const FcitxQtInputMethodEntryList &imEntryList = m_filteredIMEntryList[index.parent().row()].second;

    const FcitxQtInputMethodEntry &imEntry = imEntryList[index.row()];

    switch (role) {

    case Qt::DisplayRole:
        return imEntry.name();

    case FcitxRowTypeRole:
        return IMType;

    case FcitxIMUniqueNameRole:
        return imEntry.uniqueName();

    case FcitxLanguageRole:
        return imEntry.languageCode();

    default:
        osaLogInfo(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "NOTICE: role is not exist.\n");
    }
    return QVariant();
}

void AvailIMModel::filterIMEntryList(
    const FcitxQtInputMethodEntryList &imEntryList,
    const FcitxQtStringKeyValueList &enabledIMList) {
    beginResetModel();

    FcitxQtStringKeyValueList useIMList = getUseIMList();
    QMap<QString, int> languageMap;
    m_filteredIMEntryList.clear();

    QSet<QString> enabledIMs;
    for (const auto &item : enabledIMList) {
        enabledIMs.insert(item.key());
    }

    int idx;
    QString key;
    QString uniqueName;
    int useLanguageIndex = 0;
    int start_loc;
    for (const FcitxQtInputMethodEntry &im : imEntryList) {
        start_loc = im.name().indexOf(" - ");
        if (start_loc != -1) {
            start_loc += 3;
            int end_loc = im.name().indexOf(" - ", start_loc);
            if (end_loc != -1) {
                key = im.name().mid(start_loc, end_loc - start_loc);
            }
            else {
                key = im.name().mid(start_loc, -1);
            }
        }
        else {
            key = im.languageCode();
        }

        start_loc = key.indexOf("(");
        if (start_loc != -1) {
            key = key.mid(0, start_loc);
        }

        start_loc = key.indexOf("（");
        if (start_loc != -1) {
            key = key.mid(0, start_loc);
        }

        if (!languageMap.contains(key)) {
            idx = m_filteredIMEntryList.count();
            languageMap[key] = idx;
            m_filteredIMEntryList.append(
                QPair<QString, FcitxQtInputMethodEntryList>(key, FcitxQtInputMethodEntryList()));
            m_filteredUseIMLanguageList.append(QPair<QString, bool>(key, false));
        } else {
            idx = languageMap[key];
        }
        m_filteredIMEntryList[idx].second.append(im);

        uniqueName = im.uniqueName();

        auto iter = std::find_if(useIMList.begin(), useIMList.end(),
            [&im](const FcitxQtStringKeyValue& item) {
                return item.key() == im.uniqueName();
            });
        if (iter != useIMList.end()) {

            if (m_filteredUseIMLanguageList[idx].second == false) {
                m_filteredUseIMLanguageList[idx].second = true;
                setMaxUseIMLanguageIndex(useLanguageIndex);
                useLanguageIndex += 1;
            }
        }
    }
    endResetModel();
}

void AvailIMModel::getInputMethodEntryList(int row, FcitxQtStringKeyValueList& imNameList)
{
    FcitxQtInputMethodEntryList imEntryList = m_filteredIMEntryList[row].second;
    osaLogInfo(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "imEntryList size() [%d]\n", imEntryList.size());
    for (FcitxQtInputMethodEntry& im : imEntryList) {
        FcitxQtStringKeyValue imEntry;
        QString uniqueName = im.uniqueName();

        osaLogInfo(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "uniqueName [%s]\n", uniqueName.toStdString().c_str());

        imEntry.setKey(im.uniqueName());
        imNameList.push_back(imEntry);
    }
}

void AvailIMModel::getInputMethodEntryList(int row, FcitxQtStringKeyValueList& imNameList, FcitxQtStringKeyValueList& useIMList)
{
    FcitxQtStringKeyValueList nouseIMNameList;

    FcitxQtInputMethodEntryList imEntryList = m_filteredIMEntryList[row].second;
    for (FcitxQtInputMethodEntry& im : imEntryList) {
        FcitxQtStringKeyValue imEntry;
        QString uniqueName = im.uniqueName();

        osaLogInfo(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "uniqueName [%s]\n", uniqueName.toStdString().c_str());

        auto iter = std::find_if(useIMList.begin(), useIMList.end(),
            [&im](const FcitxQtStringKeyValue& item) {
                osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "----> item.key() [%s], im.uniqueName() [%s], = [%d]\n",
                    item.key().toStdString().c_str(), im.uniqueName().toStdString().c_str(), item.key() == im.uniqueName());
                return item.key() == im.uniqueName();
            });
        if (iter != useIMList.end()) {
            osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "----> im.uniqueName() [%s], is true. iter [%p], useIMList_.end() [%p]\n",
                im.uniqueName().toStdString().c_str(), iter, useIMList.end());

            imEntry.setKey(im.uniqueName());
            imNameList.push_back(imEntry);
        }
        else {
            imEntry.setKey(im.uniqueName());
            nouseIMNameList.push_back(imEntry);
        }
    }

    imNameList.append(nouseIMNameList);
}

void AvailIMModel::getInputMethodEntryList(int row, FcitxQtStringKeyValueList& imNameList, FcitxQtStringKeyValueList& useIMList, QString matchStr)
{
    int loc = 0;
    QString entry_name;
    FcitxQtInputMethodEntryList imEntryList = m_filteredIMEntryList[row].second;
    for (FcitxQtInputMethodEntry& im : imEntryList) {
        FcitxQtStringKeyValue imEntry;
        QString uniqueName = im.uniqueName();

        osaLogInfo(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "uniqueName [%s]\n", uniqueName.toStdString().c_str());

        auto iter = std::find_if(useIMList.begin(), useIMList.end(),
            [&im](const FcitxQtStringKeyValue& item) {
                osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "----> item.key() [%s], im.uniqueName() [%s], = [%d]\n",
                    item.key().toStdString().c_str(), im.uniqueName().toStdString().c_str(), item.key() == im.uniqueName());
                return item.key() == im.uniqueName();
            });
        if (iter != useIMList.end()) {
        }
        else {
            entry_name = im.name();
            loc = entry_name.indexOf("键盘 - ");
            if (loc != -1) {
                entry_name = entry_name.mid(loc + 5, -1);
            }

            if (entry_name.contains(matchStr, Qt::CaseInsensitive)) {
                imEntry.setKey(im.uniqueName());
                imNameList.push_back(imEntry);
            }
        }
    }
}

bool AvailIMModel::existUseIMEntryList(int row, FcitxQtStringKeyValueList& useIMList) const
{
	return m_filteredUseIMLanguageList[row].second;
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

void IMProxyModel::filterIMEntryList(const FcitxQtInputMethodEntryList &imEntryList, const FcitxQtStringKeyValueList &enabledIMList) {
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "====>\n");

    m_languageSet.clear();

    QSet<QString> enabledIMs;
    for (const auto &item : enabledIMList) {
        enabledIMs.insert(item.key());
    }

    if (enabledIMs.count() > 0) {
        for (const FcitxQtInputMethodEntry& im : imEntryList) {
            if (enabledIMs.contains(im.uniqueName())) {
                m_languageSet.insert(im.languageCode().left(2));
            }
        }
    }
    invalidate();
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "<====\n");
}

bool IMProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

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
    QString name       = index.data(Qt::DisplayRole).toString();
    QString langCode   = index.data(FcitxLanguageRole).toString();

    if (uniqueName == "keyboard-us" && m_filterText.isEmpty()) {
        return true;
    }

    bool flag = true;
    if (!m_filterText.isEmpty()) {
        flag = flag && (name.contains(m_filterText, Qt::CaseInsensitive) ||
                        languageName(langCode).contains(m_filterText,
                                                        Qt::CaseInsensitive));
    }
    return flag;
}

bool IMProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    int result = compareCategories(left, right);
    if (result < 0) {
        return true;
    }
    else if (result > 0) {
        return false;
    }

    QString l = left.data(Qt::DisplayRole).toString();
    QString r = right.data(Qt::DisplayRole).toString();
    return QCollator().compare(l, r) < 0;
}

int IMProxyModel::compareCategories(const QModelIndex &left, const QModelIndex &right) const {
    QString en_l, en_r, c_l, c_r;
    QString l_name = left.data(FcitxLanguageRole).toString();
    QString r_name = right.data(FcitxLanguageRole).toString();
    en_l = getEnglishLanguageName(l_name);
    en_r = getEnglishLanguageName(r_name);
    c_l = getChineseLanguageName(l_name);
    c_r = getChineseLanguageName(r_name);

    if (c_l == c_r) {
        return 0;
    }

    bool exist_l = left.data(FcitxUseIMLanguageRole).toBool();
    bool exist_r = right.data(FcitxUseIMLanguageRole).toBool();

    if (exist_l == true && exist_r == false) {
        return -1;
    }
    else if (exist_l == false && exist_r == true) {
        return 1;
    }

    QString loc_name = QLocale().name();
    QString code_l = left.data(FcitxLanguageCode).toString();
    QString code_r = right.data(FcitxLanguageCode).toString();
    osaLogInfo(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "loc_name [%s]\n", loc_name.toStdString().c_str());

    if (loc_name == code_l) {
        return -1;
    }
    if (loc_name == code_r) {
        return 1;
    }

    int ret = 0;
    if (loc_name == "zh" || loc_name == "zh_CN") {
        QString c_py_l = Dtk::Core::Chinese2Pinyin(c_l).toLower();
        QString c_py_r = Dtk::Core::Chinese2Pinyin(c_r).toLower();


        if (c_py_l.at(0) > c_py_r.at(0)) {
            ret = 1;
        }
        else {
            ret = -1;
        }
    }
    else {
        ret = en_l.compare(en_r);
    }
    return ret;
}

FilteredIMModel::FilteredIMModel(Mode mode, QObject *parent)
    : QAbstractListModel(parent), m_mode(mode) {}

QVariant FilteredIMModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_filteredIMEntryList.size()) {
        return QVariant();
    }

    const FcitxQtInputMethodEntry &imEntry = m_filteredIMEntryList.at(index.row());

    int loc;
    QString entry_name = "";
    switch (role) {

    case Qt::DisplayRole:
        entry_name = imEntry.name();
        loc = entry_name.indexOf("键盘 - ");
        if (loc != -1) {
            entry_name = entry_name.mid(loc + 5, -1);
        }
        return entry_name;

    case Qt::SizeHintRole:
        return QSize(0, 36);

    case FcitxRowTypeRole:
        return IMType;

    case FcitxIMUniqueNameRole:
        return imEntry.uniqueName();

    case FcitxLanguageRole:
        return imEntry.languageCode();

    case FcitxIMConfigurableRole:
        return imEntry.configurable();

    case FcitxLanguageNameRole:
        return languageName(imEntry.languageCode());

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

    case FcitxUseIMRole: {
        FcitxQtStringKeyValueList useIMList_ = getUseIMList();
        auto iter = std::find_if(useIMList_.begin(), useIMList_.end(),
            [&imEntry](const FcitxQtStringKeyValue& item) {
                osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "----> item.key() [%s], imEntry.uniqueName() [%s], = [%d]\n",
                    item.key().toStdString().c_str(), imEntry.uniqueName().toStdString().c_str(), item.key() == imEntry.uniqueName());
                return item.key() == imEntry.uniqueName();
            });
        if (iter != useIMList_.end()) {
            osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "----> imEntry.uniqueName() [%s], is true. iter [%p], useIMList_.end() [%p]\n",
                imEntry.uniqueName().toStdString().c_str(), iter, useIMList_.end());
            return true;
        }
        osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "----> imEntry.uniqueName() [%s], is false\n", imEntry.uniqueName().toStdString().c_str());
        return false;
    }

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
            {FcitxIMLayoutRole, "layout"}};
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
    emit imListChanged(m_filteredIMEntryList);
}

void FilteredIMModel::remove(int idx) {
    if (idx < 0 || idx >= m_filteredIMEntryList.size()) {
        return;
    }
    beginRemoveRows(QModelIndex(), idx, idx);
    m_filteredIMEntryList.removeAt(idx);
    endRemoveRows();
    emit imListChanged(m_filteredIMEntryList);
}
