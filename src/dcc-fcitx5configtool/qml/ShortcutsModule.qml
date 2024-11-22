// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import org.deepin.dtk 1.0 as D
import org.deepin.dcc 1.0

DccObject {
    readonly property var enumKeys: ["None", "CTRL_SHIFT", "ALT_SHIFT", "CTRL_SUPER", "ALT_SUPER"]
    property var triggerKeys: dccData.fcitx5ConfigProxy.globalConfigOption(
                                  "Hotkey", "TriggerKeys")
    property int enumerateForwardKeys: calculateEnumerateForwardKeys(
                                           dccData.fcitx5ConfigProxy.globalConfigOption(
                                               "Hotkey",
                                               "EnumerateForwardKeys").value)

    function calculateEnumerateForwardKeys(value) {
        for (var i = 0; i < value.length; i++) {
            value[i] = String(value[i]).toUpperCase().replace("META", "SUPER")
            if (value[i].endsWith("_L") || value[i].endsWith("_R")) {
                value[i] = value[i].slice(0, -2)
            }
        }
        let formattedValue = value.length > 0 ? value.join("_") : ""
        return enumKeys.indexOf(formattedValue)
    }

    // title
    DccObject {
        name: "ShortcutsTitle"
        parentName: "Manage Input Methods"
        displayName: qsTr("Shortcuts")
        weight: 210
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

    Component.onCompleted: {
        dccData.fcitx5ConfigProxy.onRequestConfigFinished.connect(() => {
                                                                      triggerKeys = dccData.fcitx5ConfigProxy.globalConfigOption(
                                                                          "Hotkey",
                                                                          "TriggerKeys")
                                                                      enumerateForwardKeys = calculateEnumerateForwardKeys(dccData.fcitx5ConfigProxy.globalConfigOption("Hotkey", "EnumerateForwardKeys").value)
                                                                  })
    }

    function reverseEnumerateForwardKeys(index) {
        if (index < 0 || index >= enumKeys.length) {
            return ""
        }
        let value = enumKeys[index]
        if (value === "None") {
            return ""
        }
        let parts = value.split("_")
        for (var i = 0; i < parts.length; i++) {
            if (parts.length > 0) {
                parts[i] = parts[i].charAt(0).toUpperCase() + parts[i].slice(
                            1).toLowerCase()
            }
            if (parts[i] === "Super") {
                parts[i] = "Meta"
            } else if (parts[i] === "Shift") {
                parts[i] = "Shift_L"
            }
        }
        return parts
    }

    // Shortcut of scroll IM
    DccObject {
        name: "scrollIM"
        parentName: "Manage Input Methods"
        weight: 220
        displayName: qsTr("Scroll between input methods")
        backgroundType: DccObject.Normal
        pageType: DccObject.Editor
        page: D.ComboBox {
            id: comboBox
            flat: true
            model: enumKeys
            currentIndex: enumerateForwardKeys

            onCurrentIndexChanged: {
                console.log("Current index changed to:", currentIndex,
                            "with text:", model[currentIndex])
                dccData.fcitx5ConfigProxy.setValue(
                            "Hotkey/EnumerateForwardKeys/0",
                            reverseEnumerateForwardKeys(currentIndex), true)
            }
        }
    }

    // Shortcut of turn on or off
    DccObject {
        name: "turnGrouup"
        parentName: "Manage Input Methods"
        weight: 230
        pageType: DccObject.Item
        page: DccGroupView {}

        DccObject {
            name: "shortcutSetting"
            parentName: "Manage Input Methods/turnGrouup"
            displayName: qsTr("Turn on or off input methods")
            weight: 231
            pageType: DccObject.Editor
            page: D.KeySequenceEdit {
                placeholderText: qsTr("Please enter a new shortcut")
                keys: triggerKeys.value
                background: null

                onKeysChanged: {
                    if (keys.length > 0) {
                        dccData.fcitx5ConfigProxy.setValue(
                                    "Hotkey/TriggerKeys/0", keys, true)
                    }
                }
            }
        }

        DccObject {
            name: "shortcutSettingDesc"
            parentName: "Manage Input Methods/turnGrouup"
            weight: 232
            pageType: DccObject.Item
            page: Label {
                leftPadding: 10
                rightPadding: 10
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
