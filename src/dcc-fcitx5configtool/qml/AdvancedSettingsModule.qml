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
        parentName: "AdvancedSettings"
        displayName: qsTr("Advanced Settings")
        weight: 310
        pageType: DccObject.Item
        onParentItemChanged: {
            if (parentItem) {
                parentItem.bottomPadding = 0
            }
        }
        page: ColumnLayout {
            spacing: 0
            Label {
                Layout.leftMargin: 10
                Layout.bottomMargin: 0
                font {
                    family: D.DTK.fontManager.t4.family
                    pixelSize: D.DTK.fontManager.t4.pixelSize
                    weight: Font.Medium
                }
                text: dccObj.displayName
            }
            Label {
                Layout.topMargin: 0
                leftPadding: 10
                rightPadding: 10
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
        parentName: "AdvancedSettings"
        displayName: qsTr("Global Config")
        weight: 320
        onParentItemChanged: {
            if (parentItem) {
                parentItem.topPadding = 6
            }
        }
        page: DccRightView {
            spacing: 5
        }
        GlobalConfigPage {}
    }

    DccObject {
        name: "AddonsPage"
        parentName: "AdvancedSettings"
        displayName: qsTr("Add-ons")
        weight: 330
        page: DccRightView {
            spacing: 0
        }
        AddonsPage {}
    }
}
