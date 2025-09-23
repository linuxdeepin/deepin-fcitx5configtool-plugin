// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import org.deepin.dtk 1.0 as D
import org.deepin.dcc 1.0

DccObject {
    readonly property var enumKeys: ["Ctrl+Shift", "Alt+Shift", "None"]
    readonly property var enumKeysI18n: ["Ctrl+Shift", "Alt+Shift", qsTr("None")]
    readonly property var triggerEnumKeys: ["Shift", "Ctrl+Space", "None"]
    readonly property var triggerEnumKeysI18n: ["Shift", "Ctrl+Space", qsTr("None")]
    
    property var triggerKeys: dccData.fcitx5ConfigProxy.globalConfigOption(
                                  "Hotkey", "TriggerKeys")
    property int enumerateForwardKeys: calculateEnumerateForwardKeys(
                                           dccData.fcitx5ConfigProxy.globalConfigOption(
                                               "Hotkey",
                                               "EnumerateForwardKeys").value)
    property int currentTriggerKeys: calculateTriggerKeys(triggerKeys.value)
    property bool isUserChanging: false

    function calculateEnumerateForwardKeys(value) {
        for (var i = 0; i < value.length; i++) {
            value[i] = String(value[i]).toUpperCase().replace("META", "SUPER")
            if (value[i].endsWith("_L") || value[i].endsWith("_R")) {
                value[i] = value[i].slice(0, -2)
            }
        }
        let formattedValue = value.length > 0 ? value.join("_") : ""

        if (formattedValue === "CTRL_SHIFT") formattedValue = "Ctrl+Shift"
        if (formattedValue === "ALT_SHIFT") formattedValue = "Alt+Shift"
        if (formattedValue === "") formattedValue = "None"  // 空配置映射到 "None"
        
        return enumKeys.indexOf(formattedValue)
    }

    function calculateTriggerKeys(value) {
        // 获取完整的TriggerKeys配置
        let triggerKeys0 = dccData.fcitx5ConfigProxy.globalConfigOption("Hotkey", "TriggerKeys").value
        let triggerKeys1 = []
        try {
            // 尝试获取第二个TriggerKey
            let triggerKeysObj = dccData.fcitx5ConfigProxy.globalConfigOption("Hotkey", "TriggerKeys")
            if (triggerKeysObj && triggerKeysObj.hasOwnProperty("1")) {
                triggerKeys1 = triggerKeysObj["1"] || []
            }
        } catch(e) {
            console.warn("Could not get TriggerKeys/1:", e)
        }
        
        // 检查是否是分别设置的左右Shift
        let hasShiftL = (triggerKeys0 && triggerKeys0.includes && triggerKeys0.includes("Shift_L")) ||
                        (triggerKeys1 && triggerKeys1.includes && triggerKeys1.includes("Shift_L"))
        let hasShiftR = (triggerKeys0 && triggerKeys0.includes && triggerKeys0.includes("Shift_R")) ||
                        (triggerKeys1 && triggerKeys1.includes && triggerKeys1.includes("Shift_R"))
        
        if (hasShiftL || hasShiftR) {
            console.log("CASE: Detected separate Shift keys, returning Shift (0)")
            return 0  // "Shift"
        }
        
        // 优先使用triggerKeys0的数据进行判断，而不是value参数
        let actualKeys = triggerKeys0 || []
        
        // 如果TriggerKeys/0配置真正为空或未定义，这是初始状态，默认使用Shift
        if (!actualKeys || actualKeys.length === 0) {
            console.log("CASE: Empty TriggerKeys/0, returning Shift (0)")
            return 0  // "Shift" - 初始默认
        }
        
        // 如果是用户主动设置的None（fcitx5格式为[""]）
        if (actualKeys.length === 1 && actualKeys[0] === "") {
            console.warn("CASE: User selected None, returning None (2)")
            return 2  // "None" - 用户主动选择的None
        }
        
        let keyStrings = actualKeys.map(key => String(key))
        
        // 检查是否是Ctrl+Space组合
        let isCtrlSpace = keyStrings.length === 2 && 
                         (keyStrings.includes("Ctrl") || keyStrings.includes("Control") || keyStrings.includes("Control_L")) && 
                         (keyStrings.includes("Space") || keyStrings.includes("space"))
        if (isCtrlSpace) {
            console.warn("CASE: Detected Ctrl+Space, returning Ctrl+Space (1)")
            return 1  // "Ctrl+Space"
        }
        return 2  // "None"
    }

    function reverseTriggerKeys(index) {
        console.warn("=== reverseTriggerKeys ===")
        console.warn("Input index:", index)
        let result
        switch(index) {
            case 0: 
                result = "Shift_L Shift_R" 
                break
            case 1: 
                result = ["Ctrl", "Space"] 
                break
            case 2: 
                result = [""]  // "None"
                break
            default:
                result = ["Shift_L", "Shift_R"]
                break
        }
        return result
    }

    // title
    DccObject {
        name: "ShortcutsTitle"
        parentName: "Shortcuts"
        displayName: qsTr("Shortcuts")
        weight: 210
        pageType: DccObject.Item
        page: RowLayout {
            Label {
                Layout.leftMargin: 10
                font {
                    family: D.DTK.fontManager.t4.family
                    pixelSize: D.DTK.fontManager.t4.pixelSize
                    weight: Font.Medium
                }
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
        // 设置翻译后的显示数组
        enumKeysDisplay = ["Ctrl+Shift", "Alt+Shift", qsTr("None")]
        triggerEnumKeysDisplay = ["Shift", "Ctrl+Space", qsTr("None")]
        
        dccData.fcitx5ConfigProxy.onRequestConfigFinished.connect(() => {
                                                                      
                                                                      let fullTriggerKeys = dccData.fcitx5ConfigProxy.globalConfigOption("Hotkey", "TriggerKeys")
                                                                      
                                                                      if (!isUserChanging) {
                                                                          triggerKeys = fullTriggerKeys
                                                                          console.warn("Updated triggerKeys:", triggerKeys.value)
                                                                      } else {
                                                                          console.warn("Skipping config update - user is changing")
                                                                      }
                                                                      enumerateForwardKeys = calculateEnumerateForwardKeys(dccData.fcitx5ConfigProxy.globalConfigOption("Hotkey", "EnumerateForwardKeys").value)
                                                                  })
    }

    function reverseEnumerateForwardKeys(index) {
        if (index < 0 || index >= enumKeys.length) {
            return ""
        }
        let value = enumKeys[index]
        
        if (value === "Ctrl+Shift") value = "CTRL_SHIFT"
        if (value === "Alt+Shift") value = "ALT_SHIFT"
        
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
        parentName: "Shortcuts"
        weight: 220
        displayName: qsTr("Scroll between input methods")
        backgroundType: DccObject.Normal
        pageType: DccObject.Editor
        page: D.ComboBox {
            id: comboBox
            flat: true
            model: enumKeysI18n
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
        parentName: "Shortcuts"
        weight: 230
        pageType: DccObject.Item
        page: DccGroupView {}

        DccObject {
            name: "shortcutSetting"
            parentName: "Shortcuts/turnGrouup"
            displayName: qsTr("Turn on or off input methods")
            weight: 231
            pageType: DccObject.Editor
            page: D.ComboBox {
                id: triggerComboBox
                flat: true
                model: triggerEnumKeysI18n
                currentIndex: currentTriggerKeys

                onCurrentIndexChanged: {
                    isUserChanging = true
                    
                    if (currentIndex === 0) {
                        // Shift选项：分别设置左右Shift
                        dccData.fcitx5ConfigProxy.setValue("Hotkey/TriggerKeys/0", ["Shift_L"], true)
                        dccData.fcitx5ConfigProxy.setValue("Hotkey/TriggerKeys/1", ["Shift_R"], true)
                    } else {
                        let newKeys = reverseTriggerKeys(currentIndex)
                        dccData.fcitx5ConfigProxy.setValue("Hotkey/TriggerKeys/0", newKeys, true)
                        dccData.fcitx5ConfigProxy.setValue("Hotkey/TriggerKeys/1", [""], true)
                    }
                    
                    Qt.callLater(() => {
                        isUserChanging = false
                    })
                }
            }
        }

        DccObject {
            name: "shortcutSettingDesc"
            parentName: "Shortcuts/turnGrouup"
            weight: 232
            pageType: DccObject.Item
            page: Label {
                topPadding: 5
                bottomPadding: 5
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
