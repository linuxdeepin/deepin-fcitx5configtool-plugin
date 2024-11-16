// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import org.deepin.dtk 1.0 as D
import org.deepin.dcc 1.0

DccObject {
    // title
    DccObject {
        name: "ShortcutsTitle"
        parentName: "Fcitx5configtool"
        displayName: qsTr("Shortcuts")
        weight: 210
        hasBackground: false
        pageType: DccObject.Item
        page: RowLayout {
            Label {
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: dccObj.displayName
            }
            D.Button {
                id: resetBtn
                text: qsTr("Restore Defaults")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.rightMargin: 10
                background: null
                textColor: D.Palette {
                    normal: D.DTK.makeColor(D.Color.Highlight)
                }
                onClicked: {
                    dccData.fcitx5ConfigProxy.restoreDefault("Hotkey")
                    console.log("Restore Defaults button clicked")
                }
            }
        }
    }

    // Shortcut of scroll IM
    DccObject {
        name: "scrollIM"
        parentName: "Fcitx5configtool"
        weight: 220
        displayName: qsTr("Scroll between input methods")
        hasBackground: true
        pageType: DccObject.Editor
        page: D.ComboBox {
            id: comboBox
            flat: true
            model: ["None", "CTRL_SHIFT", "ALT_SHIFT", "CTRL_SUPER", "ALT_SUPER"]
            currentIndex: 1 // TODO(zhangs): config

            onCurrentIndexChanged: {
                console.log("Current index changed to:", currentIndex,
                            "with text:", model[currentIndex])
            }
        }
    }

    // Shortcut of turn on or off
    DccObject {
        name: "turnGrouup"
        parentName: "Fcitx5configtool"
        weight: 230
        pageType: DccObject.Item
        page: DccGroupView {}

        DccObject {
            name: "shortcutSetting"
            parentName: "Fcitx5configtool/turnGrouup"
            displayName: qsTr("Turn on or off input methods")
            weight: 231
            pageType: DccObject.Editor
            page: D.KeySequenceEdit {
                placeholderText: qsTr("Enter a new shortcut")
                keys: ["CTRL", "SPACE"]
                background: null
            }
        }

        DccObject {
            name: "shortcutSettingDesc"
            parentName: "Fcitx5configtool/turnGrouup"
            weight: 232
            pageType: DccObject.Item
            page: Label {
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.fillWidth: true
                font: D.DTK.fontManager.t8
                opacity: 0.5
                wrapMode: Text.Wrap
                text: qsTr(
                          "It turns on or off the currently used input method."
                          + "If no input method is being used or the first input "
                          + "method is not the keyboard, it switches between the "
                          + "first input method and the currently used keyboard/input method.")
            }
        }
    }
}
