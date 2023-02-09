// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ADDIM_LAYOUTWIDGET_H
#define ADDIM_LAYOUTWIDGET_H

#include "iso639.h"
#include "layoutmodel.h"
#include "layoutprovider.h"
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QWidget>
#include <fcitxqtdbustypes.h>
#include <memory>

class QDBusPendingCallWatcher;
class DBusProvider;

namespace Ui {
class LayoutSelector;
}

namespace fcitx {
namespace addim {
class KeyboardLayoutWidget;
class LanguageFilterModel;
class LayoutInfoModel;
class VariantInfoModel;

class LayoutSelector : public QWidget {
    Q_OBJECT
public:
    LayoutSelector(DBusProvider *dbus, QWidget *parent = nullptr);
    ~LayoutSelector();
    void setLayout(const QString &layout, const QString &variant);

    static LayoutSelector *
    selectLayout(QWidget *parent, DBusProvider *dbus, const QString &title,
                 const QString &layout);

    QPair<QString, QString> layout() const;

private slots:
    void languageComboBoxChanged();
    void layoutComboBoxChanged();
    void variantComboBoxChanged();

private:
    std::unique_ptr<Ui::LayoutSelector> m_ui;
    DBusProvider *m_dbus;
    LayoutProvider *m_layoutProvider;
    KeyboardLayoutWidget *m_keyboardLayoutWidget = nullptr;
    Iso639 m_iso639;

    QString m_preSelectLayout;
    QString m_preSelectVariant;
};

} // namespace addim
} // namespace fcitx

#endif // _KCM_FCITX_LAYOUTWIDGET_H_
