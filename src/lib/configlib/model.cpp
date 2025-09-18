// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model.h"

#include "xkbrules.h"
#include "logging.h"

#include <DStyleOption>

#include <QCollator>
#include <QDebug>
#include <QLocale>
#include <QSize>
#include <QColor>

#include <fcitx-utils/i18n.h>

namespace fcitx {
namespace kcm {

CategorizedItemModel::CategorizedItemModel(QObject *parent)
    : QAbstractItemModel(parent) {
    qCDebug(KCM_FCITX5) << "Initializing CategorizedItemModel";
}

int CategorizedItemModel::rowCount(const QModelIndex &parent) const
{
    // qCDebug(KCM_FCITX5) << "rowCount called for parent:" << parent;
    if (!parent.isValid()) {
        // qCDebug(KCM_FCITX5) << "Parent is invalid, returning listSize:" << listSize();
        return listSize();
    }

    if (parent.internalId() > 0) {
        // qCDebug(KCM_FCITX5) << "Parent internalId > 0, returning 0";
        return 0;
    }

    if (parent.column() > 0 || parent.row() >= listSize()) {
        // qCWarning(KCM_FCITX5) << "Parent column > 0 or row >= listSize, returning 0. Column:" << parent.column() << "Row:" << parent.row();
        return 0;
    }
    int size = subListSize(parent.row());
    // qCDebug(KCM_FCITX5) << "Returning subListSize:" << size << "for parent row:" << parent.row();
    return size;
}

int CategorizedItemModel::columnCount(const QModelIndex &) const
{
    // qCDebug(KCM_FCITX5) << "columnCount called, returning 1";
    return 1;
}

QModelIndex CategorizedItemModel::parent(const QModelIndex &child) const
{
    // qCDebug(KCM_FCITX5) << "parent called for child:" << child;
    if (!child.isValid()) {
        // qCDebug(KCM_FCITX5) << "Child is invalid, returning empty QModelIndex";
        return QModelIndex();
    }

    int row = child.internalId();
    if (row && row - 1 >= listSize()) {
        // qCWarning(KCM_FCITX5) << "Child internalId is out of bounds, returning empty QModelIndex";
        return QModelIndex();
    }

    return createIndex(row - 1, 0, -1);
}

QModelIndex CategorizedItemModel::index(int row, int column,
                                        const QModelIndex &parent) const
{
    // qCDebug(KCM_FCITX5) << "index called for row:" << row << "column:" << column << "parent:" << parent;
    // return language index
    if (!parent.isValid()) {
        if (column > 0 || row >= listSize()) {
            // qCWarning(KCM_FCITX5) << "Parent invalid and column > 0 or row >= listSize, returning empty QModelIndex";
            return QModelIndex();
        } else {
            // qCDebug(KCM_FCITX5) << "Parent invalid, created index:" << createIndex(row, column, static_cast<quintptr>(0));
            return createIndex(row, column, static_cast<quintptr>(0));
        }
    }

    // return im index
    if (parent.column() > 0 || parent.row() >= listSize() || row >= subListSize(parent.row())) {
        // qCWarning(KCM_FCITX5) << "Parent valid but parameters out of bounds, returning empty QModelIndex";
        return QModelIndex();
    }

    // qCDebug(KCM_FCITX5) << "Parent valid, created index:" << createIndex(row, column, parent.row() + 1);
    return createIndex(row, column, parent.row() + 1);
}

QVariant CategorizedItemModel::data(const QModelIndex &index, int role) const
{
    // qCDebug(KCM_FCITX5) << "data called for index:" << index << "role:" << role;
    if (!index.isValid()) {
        // qCDebug(KCM_FCITX5) << "Invalid index, returning empty QVariant";
        return QVariant();
    }

    if (!index.parent().isValid()) {
        if (index.column() > 0 || index.row() >= listSize()) {
            // qCWarning(KCM_FCITX5) << "Data: Parent invalid and column > 0 or row >= listSize, returning empty QVariant";
            return QVariant();
        }

        return dataForCategory(index, role);
    }

    if (index.column() > 0 || index.parent().column() > 0 || index.parent().row() >= listSize()) {
        // qCWarning(KCM_FCITX5) << "Data: Parameters out of bounds, returning empty QVariant";
        return QVariant();
    }

    if (index.row() >= subListSize(index.parent().row())) {
        // qCWarning(KCM_FCITX5) << "Data: Row out of bounds for sublist, returning empty QVariant";
        return QVariant();
    }
    return dataForItem(index, role);
}

static QString languageName(const QString &uniqueName, const QString &langCode)
{
    qCDebug(KCM_FCITX5) << "languageName called for uniqueName:" << uniqueName << "langCode:" << langCode;
    const auto &xkbrules = XkbRules::instance();
    QString langName;

    if (uniqueName.startsWith("keyboard-")) {
        auto layout = xkbrules.layout(uniqueName);
        langName = layout.description;
        qCDebug(KCM_FCITX5) << "Is a keyboard layout, description:" << langName;
    }

    if (!langName.isEmpty()) {
        qCDebug(KCM_FCITX5) << "Returning translated layout description:" << langName;
        return XkbRules::tr(langName);
    }

    if (langCode.isEmpty()) {
        qCWarning(KCM_FCITX5) << "langCode is empty, returning 'Unknown'";
        return _("Unknown");
    } else if (langCode == "*") {
        qCDebug(KCM_FCITX5) << "langCode is '*', returning 'Multilingual'";
        return _("Multilingual");
    }

    langName = QLocale(langCode).nativeLanguageName();
    if (langName.isEmpty()) {
        langName = "Unknown";
        qInfo("NOTICE: uniqueName [%s] not found english name. unknown.", uniqueName.toStdString().c_str());
    }

    qCDebug(KCM_FCITX5) << "Returning native language name:" << langName;
    return langName;
}

static QString getEnglishLanguageName(const QString &uniqueName, const QString &langCode)
{
    qCDebug(KCM_FCITX5) << "getEnglishLanguageName called for uniqueName:" << uniqueName << "langCode:" << langCode;
    const auto &xkbrules = XkbRules::instance();
    QString englishName;

    if (uniqueName.startsWith("keyboard-")) {
        auto layout = xkbrules.layout(uniqueName);
        englishName = layout.description;
        qCDebug(KCM_FCITX5) << "Is a keyboard layout, English description:" << englishName;
    }

    if (!englishName.isEmpty()) {
        qCDebug(KCM_FCITX5) << "Returning layout description:" << englishName;
        return englishName;
    }

    if (langCode.isEmpty()) {
        qCWarning(KCM_FCITX5) << "English: langCode is empty, returning 'Unknown'";
        return "Unknown";
    } else if (langCode == "*") {
        qCDebug(KCM_FCITX5) << "English: langCode is '*', returning 'Multilingual'";
        return "Multilingual";
    }

    englishName = QLocale::languageToString(QLocale(langCode).language());
    if (englishName.isEmpty()) {
        englishName = "Unknown";
        qInfo("NOTICE: uniqueName [%s] not found name. unknown.", uniqueName.toStdString().c_str());
    }

    qCDebug(KCM_FCITX5) << "Returning English language name:" << englishName;
    return englishName;
}

AvailIMModel::AvailIMModel(QObject *parent)
    : CategorizedItemModel(parent) {
    qCDebug(KCM_FCITX5) << "Initializing AvailIMModel";
}

QVariant AvailIMModel::dataForCategory(const QModelIndex &index,
                                       int role) const
{
    qCDebug(KCM_FCITX5) << "dataForCategory called for index:" << index << "role:" << role;
    switch (role) {

    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        qCDebug(KCM_FCITX5) << "role is Qt::DisplayRole or Qt::ToolTipRole";
        return filteredIMEntryList[index.row()].first;

    case FcitxLanguageRole:
        qCDebug(KCM_FCITX5) << "role is FcitxLanguageRole";
        return filteredIMEntryList[index.row()].second.at(0).languageCode();

    case FcitxIMUniqueNameRole:
        qCDebug(KCM_FCITX5) << "role is FcitxIMUniqueNameRole";
        return QString();

    case FcitxRowTypeRole:
        qCDebug(KCM_FCITX5) << "role is FcitxRowTypeRole";
        return LanguageType;

    // 设置背景色
    case Dtk::ViewItemBackgroundRole:
        qCDebug(KCM_FCITX5) << "role is Dtk::ViewItemBackgroundRole";
        return QVariant::fromValue(QPair<int, int> { QPalette::Base, Dtk::Gui::DPalette::NoType });

    default:
        qCDebug(KCM_FCITX5) << "dataForCategory returning empty QVariant for unhandled role:" << role;
        return QVariant();
    }
}

QVariant AvailIMModel::dataForItem(const QModelIndex &index, int role) const
{
    qCDebug(KCM_FCITX5) << "dataForItem called for index:" << index << "role:" << role;
    const FcitxQtInputMethodEntryList &imEntryList =
            filteredIMEntryList[index.parent().row()].second;

    const FcitxQtInputMethodEntry &imEntry = imEntryList[index.row()];

    switch (role) {

    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        qCDebug(KCM_FCITX5) << "role is Qt::DisplayRole or Qt::ToolTipRole";
        return imEntry.name();

    case FcitxRowTypeRole:
        qCDebug(KCM_FCITX5) << "role is FcitxRowTypeRole";
        return IMType;

    case FcitxIMUniqueNameRole:
        qCDebug(KCM_FCITX5) << "role is FcitxIMUniqueNameRole";
        return imEntry.uniqueName();

    case FcitxLanguageRole:
        qCDebug(KCM_FCITX5) << "role is FcitxLanguageRole";
        return imEntry.languageCode();

    case FcitxIMEnabledRole:
        qCDebug(KCM_FCITX5) << "role is FcitxIMEnabledRole";
        return enabledIMs_.contains(imEntry.uniqueName());

    // 设置背景色
    case Dtk::ViewItemBackgroundRole:
        qCDebug(KCM_FCITX5) << "role is Dtk::ViewItemBackgroundRole";
        return QVariant::fromValue(QPair<int, int> { QPalette::Base, Dtk::Gui::DPalette::NoType });
    }
    qCDebug(KCM_FCITX5) << "dataForItem returning empty QVariant for unhandled role:" << role;
    return QVariant();
}

Qt::ItemFlags AvailIMModel::flags(const QModelIndex &index) const
{
    qCDebug(KCM_FCITX5) << "flags called for index:" << index;
    auto flags = CategorizedItemModel::flags(index);

    // 已被启用的输入法，置灰且不可选择
    if (index.data(FcitxRowTypeRole) == IMType) {
        bool imEnabled = index.data(FcitxIMEnabledRole).toBool();
        if (imEnabled) {
            qCDebug(KCM_FCITX5) << "IM is enabled, disabling selection for index:" << index << "flags:" << flags;
            flags &= ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        }
    }

    qCDebug(KCM_FCITX5) << "flags returning:" << flags;
    return flags;
}

static QString buildKeyByIMNameAndLanguageCode(const QString &uniqueName, const QString &languageCode)
{
    qCDebug(KCM_FCITX5) << "buildKeyByIMNameAndLanguageCode called for uniqueName:" << uniqueName << "languageCode:" << languageCode;
    auto translatedName = languageName(uniqueName, languageCode);
    auto englishName = getEnglishLanguageName(uniqueName, languageCode);
    QString key = translatedName + " - " + englishName;
    qCDebug(KCM_FCITX5) << "Built key:" << key;
    return key;
}

void AvailIMModel::filterIMEntryList(
        const FcitxQtInputMethodEntryList &imEntryList,
        const FcitxQtStringKeyValueList &enabledIMList)
{
    qCDebug(KCM_FCITX5) << "Filtering IM entry list with" << imEntryList.size() << "entries";
    beginResetModel();

    QMap<QString, int> languageMap;
    filteredIMEntryList.clear();

    enabledIMs_.clear();
    for (const auto &item : enabledIMList) {
        enabledIMs_.insert(item.key());
    }

    for (const FcitxQtInputMethodEntry &im : imEntryList) {
        QString key = buildKeyByIMNameAndLanguageCode(im.uniqueName(), im.languageCode());
        int idx;
        if (!languageMap.contains(key)) {
            idx = filteredIMEntryList.count();
            languageMap[key] = idx;
            filteredIMEntryList.append(
                    QPair<QString, FcitxQtInputMethodEntryList>(
                            key, FcitxQtInputMethodEntryList()));
        } else {
            idx = languageMap[key];
        }
        filteredIMEntryList[idx].second.append(im);
    }

    endResetModel();
    qCInfo(KCM_FCITX5) << "AvailIMModel::filterIMEntryList finished. Filtered list now contains" << filteredIMEntryList.size() << "categories.";
}

IMProxyModel::IMProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    qCDebug(KCM_FCITX5) << "Initializing IMProxyModel";
    setDynamicSortFilter(true);
    sort(0);
}

void IMProxyModel::setFilterText(const QString &text)
{
    qCInfo(KCM_FCITX5) << "Setting filter text to:" << text;
    if (filterText_ != text) {
        qCDebug(KCM_FCITX5) << "filter is not text:" << text;
        filterText_ = text;
        invalidate();
    }
}

void IMProxyModel::setShowOnlyCurrentLanguage(bool show)
{
    qCInfo(KCM_FCITX5) << "Setting show only current language to:" << show;
    if (showOnlyCurrentLanguage_ != show) {
        qCDebug(KCM_FCITX5) << "showOnlyCurrentLanguage is not show:" << show;
        showOnlyCurrentLanguage_ = show;
        invalidate();
    }
}

void IMProxyModel::filterIMEntryList(
        const FcitxQtInputMethodEntryList &imEntryList,
        const FcitxQtStringKeyValueList &enabledIMList)
{
    // qCInfo(KCM_FCITX5) << "IMProxyModel::filterIMEntryList started.";
    languageSet_.clear();

    QSet<QString> enabledIMs;
    for (const auto &item : enabledIMList) {
        enabledIMs.insert(item.key());
    }
    for (const FcitxQtInputMethodEntry &im : imEntryList) {
        if (enabledIMs.contains(im.uniqueName())) {
            languageSet_.insert(im.languageCode().left(2));
        }
    }
    invalidate();
    // qCInfo(KCM_FCITX5) << "IMProxyModel::filterIMEntryList finished.";
}

bool IMProxyModel::filterAcceptsRow(int source_row,
                                    const QModelIndex &source_parent) const
{
    qCDebug(KCM_FCITX5) << "filterAcceptsRow called for source_row:" << source_row << "source_parent:" << source_parent;
    const QModelIndex index =
            sourceModel()->index(source_row, 0, source_parent);

    if (index.data(FcitxRowTypeRole) == LanguageType) {
        return filterLanguage(index);
    }

    return filterIM(index);
}

bool IMProxyModel::filterLanguage(const QModelIndex &index) const
{
    qCDebug(KCM_FCITX5) << "filterLanguage called for index:" << index;
    if (!index.isValid()) {
        qCWarning(KCM_FCITX5) << "filterLanguage: Invalid index";
        return false;
    }

    int childCount = index.model()->rowCount(index);
    if (childCount == 0) {
        qCDebug(KCM_FCITX5) << "filterLanguage: No children, returning false";
        return false;
    }

    for (int i = 0; i < childCount; ++i) {
        if (filterIM(index.model()->index(i, 0, index))) {
            qCDebug(KCM_FCITX5) << "filterLanguage: Child" << i << "is accepted, returning true";
            return true;
        }
    }

    qCDebug(KCM_FCITX5) << "filterLanguage: No child accepted, returning false";
    return false;
}

bool IMProxyModel::filterIM(const QModelIndex &index) const
{
    qCDebug(KCM_FCITX5) << "filterIM called for index:" << index;
    QString uniqueName = index.data(FcitxIMUniqueNameRole).toString();
    QString name = index.data(Qt::DisplayRole).toString();
    QString langCode = index.data(FcitxLanguageRole).toString();

    // Always show keyboard us if we are not searching.
    if (uniqueName == "keyboard-us" && filterText_.isEmpty()) {
        qCDebug(KCM_FCITX5) << "filterIM: keyboard-us is always shown when not searching.";
        return true;
    }

    bool flag = true;
    QString lang = langCode.left(2);
    bool showOnlyCurrentLanguage =
            filterText_.isEmpty() && showOnlyCurrentLanguage_;

    flag =
            flag && (showOnlyCurrentLanguage ? !lang.isEmpty() && (QLocale().name().startsWith(lang) || languageSet_.contains(lang)) : true);
    if (!filterText_.isEmpty()) {
        flag = flag && (name.contains(filterText_, Qt::CaseInsensitive) || uniqueName.contains(filterText_, Qt::CaseInsensitive) || langCode.contains(filterText_, Qt::CaseInsensitive) || languageName(uniqueName, langCode).contains(filterText_, Qt::CaseInsensitive));
    }
    qCDebug(KCM_FCITX5) << "filterIM: Final result for" << uniqueName << "is" << flag;
    return flag;
}

bool IMProxyModel::lessThan(const QModelIndex &left,
                            const QModelIndex &right) const
{
    // qCDebug(KCM_FCITX5) << "lessThan called for left:" << left.data() << "right:" << right.data();
    // 先获取左右两项的类型
    bool isLeftKeyboard = left.data(FcitxIMUniqueNameRole).toString().startsWith("keyboard-");
    bool isRightKeyboard = right.data(FcitxIMUniqueNameRole).toString().startsWith("keyboard-");

    // 如果一个是键盘一个是输入法，输入法永远排在前面
    if (isLeftKeyboard != isRightKeyboard) {
        // qCDebug(KCM_FCITX5) << "lessThan: One is keyboard, one is IM. Left is keyboard:" << isLeftKeyboard;
        return !isLeftKeyboard;   // 返回true表示left排在前面，所以非键盘(输入法)应该返回true
    }

    // 如果都是同类型（都是键盘或都是输入法），按原有逻辑排序
    if (left.data(FcitxRowTypeRole) == LanguageType) {
        int result = compareCategories(left, right);
        // qCDebug(KCM_FCITX5) << "lessThan: Comparing categories, result:" << result;
        if (result < 0) {
            return true;
        } else if (result > 0) {
            return false;
        }
    } else {
        int result = compareItems(left, right);
        // qCDebug(KCM_FCITX5) << "lessThan: Comparing items, result:" << result;
        if (result < 0) {
            return true;
        } else if (result > 0) {
            return false;
        }
    }

    // 最后按显示名称排序
    QString l = left.data(Qt::DisplayRole).toString();
    QString r = right.data(Qt::DisplayRole).toString();
    return QCollator().compare(l, r) < 0;
}

enum class CONTAIN_CUR_LANG {
    NO,
    YES,
    DIFF_VARIANT,
};

static std::tuple<bool, CONTAIN_CUR_LANG> checkCategories(QAbstractItemModel *model, const QModelIndex &index)
{
    qCDebug(KCM_FCITX5) << "checkCategories called for index:" << index;
    bool containsEnabledIM = false;
    auto containsCurrentLanguage = CONTAIN_CUR_LANG::NO;

    auto childrenCount = model->rowCount(index);
    for (int r = 0; r < childrenCount; r++) {
        auto i = model->index(r, 0, index);
        bool enabled = i.data(FcitxIMEnabledRole).toBool();
        if (enabled) {
            containsEnabledIM = true;
        }

        QString lang = i.data(FcitxLanguageRole).toString();
        if (lang.isEmpty()) {
            continue;
        }

        if (QLocale().name() == lang) {
            containsCurrentLanguage = CONTAIN_CUR_LANG::YES;
        }

        if (containsCurrentLanguage == CONTAIN_CUR_LANG::NO) {
            if (QLocale().name().startsWith(lang.left(2))) {
                containsCurrentLanguage = CONTAIN_CUR_LANG::DIFF_VARIANT;
            }
        }
    }
    qCDebug(KCM_FCITX5) << "checkCategories result: containsEnabledIM:" << containsEnabledIM << "containsCurrentLanguage:" << static_cast<int>(containsCurrentLanguage);
    return { containsEnabledIM, containsCurrentLanguage };
}

int IMProxyModel::compareCategories(const QModelIndex &left,
                                    const QModelIndex &right) const
{
    // qCDebug(KCM_FCITX5) << "compareCategories called for left:" << left.data() << "right:" << right.data();
    bool leftContainsEnabledIM = false;
    auto leftContainsCurrentLanguage = CONTAIN_CUR_LANG::NO;
    std::tie(leftContainsEnabledIM, leftContainsCurrentLanguage) = checkCategories(sourceModel(), left);

    bool rightContainsEnabledIM = false;
    auto rightContainsCurrentLanguage = CONTAIN_CUR_LANG::NO;
    std::tie(rightContainsEnabledIM, rightContainsCurrentLanguage) = checkCategories(sourceModel(), right);

    if (leftContainsEnabledIM != rightContainsEnabledIM) {
        // qCDebug(KCM_FCITX5) << "compareCategories: enabledIM mismatch";
        return leftContainsEnabledIM ? -1 : 1;
    }

    if (leftContainsCurrentLanguage == rightContainsCurrentLanguage) {
        // qCDebug(KCM_FCITX5) << "compareCategories: same language containment";
        return 0;
    }

    if (leftContainsCurrentLanguage == CONTAIN_CUR_LANG::YES) {
        // qCDebug(KCM_FCITX5) << "compareCategories: left has current language";
        return -1;
    }

    if (rightContainsCurrentLanguage == CONTAIN_CUR_LANG::YES) {
        // qCDebug(KCM_FCITX5) << "compareCategories: right has current language";
        return 1;
    }

    if (leftContainsCurrentLanguage == CONTAIN_CUR_LANG::DIFF_VARIANT) {
        // qCDebug(KCM_FCITX5) << "compareCategories: left has different variant of current language";
        return -1;
    }

    if (rightContainsCurrentLanguage == CONTAIN_CUR_LANG::DIFF_VARIANT) {
        // qCDebug(KCM_FCITX5) << "compareCategories: right has different variant of current language";
        return 1;
    }
    // qCDebug(KCM_FCITX5) << "compareCategories: no difference";
    return 0;
}

int IMProxyModel::compareItems(const QModelIndex &left,
                               const QModelIndex &right) const
{
    // qCDebug(KCM_FCITX5) << "compareItems called for left:" << left.data() << "right:" << right.data();
    bool leftEnabled = left.data(FcitxIMEnabledRole).toBool();
    bool rightEnabled = right.data(FcitxIMEnabledRole).toBool();

    if (leftEnabled == rightEnabled) {
        // qCDebug(KCM_FCITX5) << "compareItems: both have same enabled state.";
        return 0;
    }

    // qCDebug(KCM_FCITX5) << "compareItems: enabled state differs. left enabled:" << leftEnabled;
    if (leftEnabled) {
        return -1;
    }

    return 1;
}

FilteredIMModel::FilteredIMModel(Mode mode, QObject *parent)
    : QAbstractListModel(parent), mode_(mode) {
    // qCDebug(KCM_FCITX5) << "Initializing FilteredIMModel with mode:" << mode;
}

QVariant FilteredIMModel::data(const QModelIndex &index, int role) const
{
    // qCDebug(KCM_FCITX5) << "FilteredIMModel::data called for index:" << index << "role:" << role;
    if (!index.isValid() || index.row() >= filteredIMEntryList_.size()) {
        // qCWarning(KCM_FCITX5) << "FilteredIMModel::data: invalid index or row out of bounds.";
        return QVariant();
    }

    const FcitxQtInputMethodEntry &imEntry =
            filteredIMEntryList_.at(index.row());

    switch (role) {

    case Qt::DisplayRole:
        // qCDebug(KCM_FCITX5) << "role is Qt::DisplayRole";
        return imEntry.name();

    case FcitxRowTypeRole:
        // qCDebug(KCM_FCITX5) << "role is FcitxRowTypeRole";
        return IMType;

    case FcitxIMUniqueNameRole:
        // qCDebug(KCM_FCITX5) << "role is FcitxIMUniqueNameRole";
        return imEntry.uniqueName();

    case FcitxLanguageRole:
        // qCDebug(KCM_FCITX5) << "role is FcitxLanguageRole";
        return imEntry.languageCode();

    case FcitxIMConfigurableRole:
        // qCDebug(KCM_FCITX5) << "role is FcitxIMConfigurableRole";
        return imEntry.configurable();

    case FcitxLanguageNameRole:
        // qCDebug(KCM_FCITX5) << "role is FcitxLanguageNameRole";
        return languageName(imEntry.uniqueName(), imEntry.languageCode());

    case FcitxIMLayoutRole: {
        // qCDebug(KCM_FCITX5) << "role is FcitxIMLayoutRole";
        auto iter = std::find_if(enabledIMList_.begin(), enabledIMList_.end(),
                                 [&imEntry](const FcitxQtStringKeyValue &item) {
                                     return item.key() == imEntry.uniqueName();
                                 });
        if (iter != enabledIMList_.end()) {
            return iter->value();
        }
        // qCDebug(KCM_FCITX5) << "no layout found for" << imEntry.uniqueName();
        return QString();
    }
    case FcitxIMActiveRole:
        // qCDebug(KCM_FCITX5) << "role is FcitxIMActiveRole";
        return index.row() > 0 ? QString("active") : QString("inactive");

    default:
        // qCDebug(KCM_FCITX5) << "unhandled role" << role;
        return QVariant();
    }
}

int FilteredIMModel::rowCount(const QModelIndex &parent) const
{
    // qCDebug(KCM_FCITX5) << "FilteredIMModel::rowCount called for parent:" << parent;
    if (parent.isValid()) {
        // qCWarning(KCM_FCITX5) << "parent is valid, returning 0.";
        return 0;
    }
    // qCDebug(KCM_FCITX5) << "FilteredIMModel::rowCount returning" << filteredIMEntryList_.count();
    return filteredIMEntryList_.count();
}

QHash<int, QByteArray> FilteredIMModel::roleNames() const
{
    // qCDebug(KCM_FCITX5) << "FilteredIMModel::roleNames called.";
    return { { Qt::DisplayRole, "name" },
             { FcitxIMUniqueNameRole, "uniqueName" },
             { FcitxLanguageRole, "languageCode" },
             { FcitxLanguageNameRole, "language" },
             { FcitxIMConfigurableRole, "configurable" },
             { FcitxIMLayoutRole, "layout" },
             { FcitxIMActiveRole, "active" } };
}

void FilteredIMModel::filterIMEntryList(
        const FcitxQtInputMethodEntryList &imEntryList,
        const FcitxQtStringKeyValueList &enabledIMList)
{
    qCDebug(KCM_FCITX5) << "Filtering IM entry list with mode:" << mode_
             << "and" << imEntryList.size() << "entries";
    beginResetModel();

    filteredIMEntryList_.clear();
    enabledIMList_ = enabledIMList;

    // We implement this twice for following reasons:
    // 1. "enabledIMs" is usually very small.
    // 2. CurrentIM mode need to keep order by enabledIMs.
    if (mode_ == CurrentIM) {
        qCDebug(KCM_FCITX5) << "Filtering for CurrentIM mode.";
        int row = 0;
        QMap<QString, const FcitxQtInputMethodEntry *> nameMap;
        for (auto &imEntry : imEntryList) {
            nameMap.insert(imEntry.uniqueName(), &imEntry);
        }

        for (const auto &im : enabledIMList) {
            if (auto value = nameMap.value(im.key(), nullptr)) {
                filteredIMEntryList_.append(*value);
                row++;
            }
        }
    } else if (mode_ == AvailIM) {
        qCDebug(KCM_FCITX5) << "Filtering for AvailIM mode.";
        QSet<QString> enabledIMs;
        for (const auto &item : enabledIMList) {
            enabledIMs.insert(item.key());
        }

        for (const FcitxQtInputMethodEntry &im : imEntryList) {
            if (enabledIMs.contains(im.uniqueName())) {
                continue;
            }
            filteredIMEntryList_.append(im);
        }
    }
    endResetModel();
    qCInfo(KCM_FCITX5) << "FilteredIMModel::filterIMEntryList finished. Filtered list now contains" << filteredIMEntryList_.size() << "items.";
}

void FilteredIMModel::move(int from, int to)
{
    qCDebug(KCM_FCITX5) << "Moving IM from" << from << "to" << to;
    if (from < 0 || from >= filteredIMEntryList_.size() || to < 0 || to >= filteredIMEntryList_.size()) {
        qCWarning(KCM_FCITX5) << "Invalid move positions:" << from << "->" << to;
        return;
    }
    beginMoveRows(QModelIndex(), from, from, QModelIndex(),
                  to > from ? to + 1 : to);
    filteredIMEntryList_.move(from, to);
    endMoveRows();
    qCInfo(KCM_FCITX5) << "Finished moving item from" << from << "to" << to;
    Q_EMIT imListChanged(filteredIMEntryList_);
}

void FilteredIMModel::remove(int idx)
{
    qCDebug(KCM_FCITX5) << "Removing IM at index:" << idx;
    if (idx < 0 || idx >= filteredIMEntryList_.size()) {
        qCWarning(KCM_FCITX5) << "Invalid remove index:" << idx;
        return;
    }
    beginRemoveRows(QModelIndex(), idx, idx);
    filteredIMEntryList_.removeAt(idx);
    endRemoveRows();
    qCInfo(KCM_FCITX5) << "Finished removing item at index" << idx;
    Q_EMIT imListChanged(filteredIMEntryList_);
}

}   // namespace kcm
}   // namespace fcitx
