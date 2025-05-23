// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xkbrules.h"
#include "logging.h"

#include <QFile>
#include <QDir>
#include <QDomDocument>
#include <libintl.h>

#include "config.h"

XkbRules &XkbRules::instance() {
    static XkbRules rules;
    static bool initialied = false;
    if (!initialied) {
        qCDebug(KCM_FCITX5) << "Initializing XkbRules instance";
        initialied = true;
        QString dir = QDir::cleanPath(QStringLiteral(XKEYBOARDCONFIG_XKBBASE) + QDir::separator() + "rules");
        qCDebug(KCM_FCITX5) << "Loading XKB rules from directory:" << dir;
        
        bool mainLoaded = rules.open(QString("%1/%2.xml").arg(dir).arg(DEFAULT_XKB_RULES));
        bool extrasLoaded = rules.open(QString("%1/%2.extras.xml").arg(dir).arg(DEFAULT_XKB_RULES));
        
        qCDebug(KCM_FCITX5) << "XKB rules loaded - main:" << mainLoaded
               << "extras:" << extrasLoaded;
    }
    return rules;
}

bool XkbRules::open(const QString &filename) {
    qCDebug(KCM_FCITX5) << "Opening XKB rules file:" << filename;
    QFile xmlFile(filename);
    if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(KCM_FCITX5) << "Failed to open XKB rules file:" << filename
                  << "error:" << xmlFile.errorString();
        return false;
    }

    QDomDocument xmlReader;
    xmlReader.setContent(&xmlFile);
    auto layoutList = xmlReader.documentElement().firstChildElement("layoutList");
    int layoutCount = 0;
    for (auto layout = layoutList.firstChildElement("layout"); !layout.isNull();
         layout = layout.nextSiblingElement("layout")) {
        layoutCount++;
        auto layoutConfigItem = layout.firstChildElement("configItem");
        auto variantList = layout.firstChildElement("variantList");

        QString layoutName = layoutConfigItem.firstChildElement("name").text();
        QString layoutShortDescription = layoutConfigItem.firstChildElement("shortDescription").text();
        QString layoutDescription = layoutConfigItem.firstChildElement("description").text();

        auto localeName = QString("%1_%2").arg(layoutShortDescription).arg(layoutName.toUpper());

        auto keyboard = QString("keyboard-%1").arg(layoutName);
        m_keyboardLayoutsMap[keyboard] = {layoutName, layoutShortDescription, layoutDescription};

        for (auto variant = variantList.firstChildElement("variant"); !variant.isNull();
             variant = variant.nextSiblingElement("variant")) {
            auto variantConfigItem = variant.firstChildElement("configItem");
            auto variantName = variantConfigItem.firstChildElement("name").text();
            auto variantShortDescription = variantConfigItem.firstChildElement("shortDescription").text();
            auto variantDescription = variantConfigItem.firstChildElement("description").text();

            auto keyboard = QString("keyboard-%1-%2").arg(layoutName).arg(variantName);

            m_keyboardLayoutsMap[keyboard] = {layoutName, layoutShortDescription, layoutDescription};
        }
    }

    qCDebug(KCM_FCITX5) << "Loaded" << layoutCount << "layouts from" << filename;
    return true;
}

QString XkbRules::tr(const QString &text) {
    return dgettext("xkeyboard-config", text.toStdString().data());
}
