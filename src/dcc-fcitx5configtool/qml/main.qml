// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import org.deepin.dcc 1.0
import org.deepin.dtk 1.0 as D

DccObject {
    DccObject {
        name: "ManageInputMethodsTitle"
        parentName: "Fcitx5configtool"
        displayName: qsTr("Manage input methods")
        weight: 10
        hasBackground: false
        pageType: DccObject.Item
        page: RowLayout {
            Label {
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: dccObj.displayName
            }
        }
    }

    DccObject {
        name: "ShortcutsTitle"
        parentName: "Fcitx5configtool"
        displayName: qsTr("Shortcuts")
        weight: 20
        hasBackground: false
        pageType: DccObject.Item
        page: RowLayout {
            Label {
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: dccObj.displayName
            }
        }
    }

    DccObject {
        name: "AdvancedSettingsTitle"
        parentName: "Fcitx5configtool"
        displayName: qsTr("Advanced Settings")
        weight: 30
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
                Layout.fillWidth: tru
                font: D.DTK.fontManager.t8
                text: qsTr("\"Advanced Settings\" is only valid for the input method that uses the system settings, if the input method has its own settings, its own settings shall prevail.")
                opacity: 0.5
                wrapMode: Text.Wrap
            }
        }
    }

    DccObject {
        name: "GlobalConfigPage"
        parentName: "Fcitx5configtool"
        displayName: qsTr("Global Config")
        weight: 31
        page: DccRightView {
            spacing: 5
        }
        GlobalConfigPage {}
    }
    DccObject {
        name: "AddonsPage"
        parentName: "Fcitx5configtool"
        displayName: qsTr("Add-ons")
        weight: 32
        page: DccRightView {
            spacing: 5
        }
        AddonsPage {}
    }
}
