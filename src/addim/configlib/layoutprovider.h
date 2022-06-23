/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _CONFIGLIB_LAYOUTPROVIDER_H_
#define _CONFIGLIB_LAYOUTPROVIDER_H_

#include "iso639.h"
#include "layoutmodel.h"
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStringListModel>
#include <fcitxqtdbustypes.h>

class QDBusPendingCallWatcher;
class DBusProvider;

namespace fcitx {
namespace addim {

class LanguageFilterModel;
class LayoutInfoModel;
class VariantInfoModel;

class LayoutProvider : public QObject {
    Q_OBJECT
    Q_PROPERTY(
        fcitx::addim::LanguageModel *languageModel READ languageModel CONSTANT)
    Q_PROPERTY(LanguageFilterModel *layoutModel READ layoutModel CONSTANT)
    Q_PROPERTY(LanguageFilterModel *variantModel READ variantModel CONSTANT)
public:
    LayoutProvider(DBusProvider *dbus, QObject *parent = nullptr);
    ~LayoutProvider();

    auto languageModel() const { return m_languageModel; }
    auto layoutModel() const { return m_layoutFilterModel; }
    auto variantModel() const { return m_variantFilterModel; }

    Q_INVOKABLE int layoutIndex(const QString &layoutString);
    Q_INVOKABLE int variantIndex(const QString &layoutString);
    Q_INVOKABLE QString layoutDescription(const QString &layoutString);

    Q_INVOKABLE void setVariantInfo(const FcitxQtLayoutInfo &info) const {
        m_variantModel->setVariantInfo(info);
    }

    Q_INVOKABLE QString layout(int layoutIdx, int variantIdx) const {
        auto layoutModelIndex = m_layoutFilterModel->index(layoutIdx, 0);
        auto variantModelIndex = m_variantFilterModel->index(variantIdx, 0);
        if (layoutModelIndex.isValid() && variantModelIndex.isValid()) {
            auto layout = layoutModelIndex.data(Qt::UserRole).toString();
            auto variant = variantModelIndex.data(Qt::UserRole).toString();
            if (layout.isEmpty()) {
                return QString();
            }
            if (variant.isEmpty()) {
                return layout;
            }
            return QString("%1-%2").arg(layout, variant);
        }
        return QString();
    }

    bool loaded() const { return m_loaded; }

signals:
    void loadedChanged();

private slots:
    void availabilityChanged();
    void fetchLayoutFinished(QDBusPendingCallWatcher *watcher);

private:
    void setLoaded(bool loaded) {
        if (loaded != m_loaded) {
            m_loaded = loaded;
            emit loadedChanged();
        }
    }

    DBusProvider *m_dbus;
    bool m_loaded = false;
    LanguageModel *m_languageModel;
    LayoutInfoModel *m_layoutModel;
    VariantInfoModel *m_variantModel;
    LanguageFilterModel *m_layoutFilterModel;
    LanguageFilterModel *m_variantFilterModel;
    Iso639 m_iso639;
};

} // namespace addim
} // namespace fcitx

#endif // _CONFIGLIB_LAYOUTPROVIDER_H_
