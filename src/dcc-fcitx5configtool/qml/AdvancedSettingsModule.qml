// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import org.deepin.dtk 1.0 as D
import org.deepin.dcc 1.0

DccObject {
    DccObject {
        name: "AdvancedSettingsTitle"
        parentName: "Fcitx5configtool"
        displayName: qsTr("Advanced Settings")
        weight: 310
        hasBackground: false
        pageType: DccObject.Item
        page: ColumnLayout {
            Label {
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: dccObj.displayName
            }
            Label {
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.fillWidth: true
                font: D.DTK.fontManager.t8
                text: qsTr(
                          "\"Advanced Settings\" is only valid for the input method that uses "
                          + "the system settings, if the input method has its own settings, "
                          + "its own settings shall prevail.")
                opacity: 0.5
                wrapMode: Text.Wrap
            }
        }
    }

    DccObject {
        name: "GlobalConfigPage"
        parentName: "Fcitx5configtool"
        displayName: qsTr("Global Config")
        weight: 320
        page: DccRightView {
            spacing: 5
        }
        GlobalConfigPage {}
    }

    DccObject {
        name: "AddonsPage"
        parentName: "Fcitx5configtool"
        displayName: qsTr("Add-ons")
        weight: 330
        page: DccRightView {
            spacing: 5
        }
        AddonsPage {}
    }
}
