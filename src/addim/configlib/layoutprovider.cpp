// SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#include "layoutprovider.h"
#include "dbusprovider.h"

namespace fcitx {
namespace addim {

LayoutProvider::LayoutProvider(DBusProvider *dbus, QObject *parent)
    : QObject(parent), m_dbus(dbus), m_languageModel(new LanguageModel(this)),
      m_layoutModel(new LayoutInfoModel(this)),
      m_variantModel(new VariantInfoModel(this)),
      m_layoutFilterModel(new LanguageFilterModel(this)),
      m_variantFilterModel(new LanguageFilterModel(this))
{
    m_layoutFilterModel->setSourceModel(m_layoutModel);
    m_variantFilterModel->setSourceModel(m_variantModel);

    connect(dbus, &DBusProvider::availabilityChanged, this,
            &LayoutProvider::availabilityChanged);
    availabilityChanged();
}

LayoutProvider::~LayoutProvider() {}

void LayoutProvider::availabilityChanged() {
    setLoaded(false);
    if (!m_dbus->controller()) {
        return;
    }

    auto call = m_dbus->controller()->AvailableKeyboardLayouts();
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            &LayoutProvider::fetchLayoutFinished);
}

void LayoutProvider::fetchLayoutFinished(QDBusPendingCallWatcher *watcher) {
    watcher->deleteLater();
    QDBusPendingReply<FcitxQtLayoutInfoList> reply = *watcher;
    if (reply.isError()) {
        return;
    }
    QSet<QString> languages;
    auto layoutInfo = reply.value();
    for (const auto &layout : layoutInfo) {
        for (const auto &language : layout.languages()) {
            languages << language;
        }
        for (const auto &variant : layout.variants()) {
            for (const auto &language : variant.languages()) {
                languages << language;
            }
        }
    }
    QStringList languageList;
    for (const auto &language : languages) {
        languageList << language;
    }
    languageList.sort();
    m_languageModel->clear();

    QStandardItem *item = new QStandardItem(_("Any language"));
    item->setData("", Qt::UserRole);
    m_languageModel->append(_("Any language"), "");
    for (const auto &language : languageList) {
        QString languageName = m_iso639.query(language);
        if (languageName.isEmpty()) {
            languageName = language;
        } else {
            languageName = QString(_("%1 (%2)")).arg(languageName, language);
        }
        m_languageModel->append(languageName, language);
    }
    m_layoutModel->setLayoutInfo(std::move(layoutInfo));
    setLoaded(true);
}

int LayoutProvider::layoutIndex(const QString &layoutString) {
    auto dashPos = layoutString.indexOf("-");
    QString layout;
    if (dashPos >= 0) {
        layout = layoutString.left(dashPos);
    } else {
        layout = layoutString;
    }
    auto &info = m_layoutModel->layoutInfo();
    auto iter = std::find_if(info.begin(), info.end(),
                             [&layout](const FcitxQtLayoutInfo &info) {
                                 return info.layout() == layout;
                             });
    if (iter != info.end()) {
        auto row = std::distance(info.begin(), iter);
        return m_layoutFilterModel->mapFromSource(m_layoutModel->index(row))
            .row();
    }
    return 0;
}

int LayoutProvider::variantIndex(const QString &layoutString) {
    auto dashPos = layoutString.indexOf("-");
    QString variant;
    if (dashPos >= 0) {
        variant = layoutString.mid(dashPos + 1);
    }
    auto &vinfo = m_variantModel->variantInfo();
    auto iter = std::find_if(vinfo.begin(), vinfo.end(),
                             [&variant](const FcitxQtVariantInfo &info) {
                                 return info.variant() == variant;
                             });
    if (iter != vinfo.end()) {
        auto row = std::distance(vinfo.begin(), iter);
        return m_variantFilterModel->mapFromSource(m_variantModel->index(row))
            .row();
    }
    return 0;
}

QString LayoutProvider::layoutDescription(const QString &layoutString) {
    auto dashPos = layoutString.indexOf("-");
    QString layout;
    QString variant;
    if (dashPos >= 0) {
        layout = layoutString.left(dashPos);
        variant = layoutString.mid(dashPos + 1);
    } else {
        layout = layoutString;
    }
    auto &info = m_layoutModel->layoutInfo();
    auto iter = std::find_if(info.begin(), info.end(),
                             [&layout](const FcitxQtLayoutInfo &info) {
                                 return info.layout() == layout;
                             });
    if (iter == info.end()) {
        return QString();
    }

    if (variant.isEmpty()) {
        return iter->description();
    }

    auto variantIter =
        std::find_if(iter->variants().begin(), iter->variants().end(),
                     [&variant](const FcitxQtVariantInfo &info) {
                         return info.variant() == variant;
                     });
    if (variantIter == iter->variants().end()) {
        return iter->description();
    }
    return QString(_("%1 - %2"))
        .arg(iter->description(), variantIter->description());
}

} // namespace addim
} // namespace fcitx
