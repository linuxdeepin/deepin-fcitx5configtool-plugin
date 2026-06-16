// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.3
import QtQuick.Window 2.15
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS
import org.deepin.dcc 1.0

DccObject {
    id: root
    property var configOptions: []
    property var configValues: ({})
    property var keyName: ""
    property bool loading: true

    // Hidden config items (fcitx5 core shortcuts)
    readonly property var hiddenConfigNames: [
        "EnumerateGroupForwardKeys",
        "EnumerateGroupBackwardKeys",
        "ActivateKeys",
        "DeactivateKeys"
    ]

    function buildValuesMap(options) {
        var vals = {}
        for (var i = 0; i < options.length; i++) {
            vals[options[i].name] = options[i].value
        }
        return vals
    }

    function filterConfigOptions(options) {
        return options.filter(function(opt) {
            return opt.name && !hiddenConfigNames.includes(opt.name)
        })
    }

    DccObject {
        id: containerItem
        name: root.displayName + "_Container"  // Add suffix to avoid conflict with root.name
        parentName: "GlobalConfigPage"
        weight: 40
        pageType: DccObject.Item
        onParentItemChanged: {
            if (parentItem) {
                parentItem.topPadding = 5
                parentItem.bottomPadding = 0
            }
        }
        page: DccGroupView {
            spacing: 0
            height: implicitHeight + 30
        }

        DccObject {
            id: headerItem
            property bool expanded: false
            parentName: containerItem.name
            displayName: root.displayName
            weight: root.weight
            pageType: DccObject.Item
            backgroundType: DccObject.Normal | DccObject.Hover
            page: RowLayout {
                id: headerRow
                height: 40
                spacing: 0

                Label {
                    Layout.leftMargin: 10
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    font: D.DTK.fontManager.t6
                    text: dccObj.displayName
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: headerItem.expanded = !headerItem.expanded
                }
                D.ToolButton {
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    icon.width: 12
                    icon.height: 12
                    icon.name: headerItem.expanded ? "arrow_ordinary_down" : "arrow_ordinary_right"
                    background: Item {}
                    onClicked: headerItem.expanded = !headerItem.expanded
                }
            }
        }

        // Must be a direct child of containerItem so DccManager can register it.
        DccRepeater {
            id: optionRepeater
            visible: !root.loading
            model: filterConfigOptions(root.configOptions)
            delegate: Component {
                DccObject {
                    id: optionDelegate
                    parentName: containerItem.name
                    displayName: modelData.description
                    weight: root.weight + index + 1
                    pageType: DccObject.Editor
                    visible: headerItem.expanded
                    backgroundType: DccObject.Normal | DccObject.Hover

                    property string optName: modelData.name
                    property string optType: modelData.type
                    property var optValue: root.configValues[optName] !== undefined
                                           ? root.configValues[optName]
                                           : modelData.value

                    page: Loader {
                        height: 40
                        sourceComponent: {
                            switch (optionDelegate.optType) {
                            case "Boolean":
                                return booleanComponent
                            case "Integer":
                                return integerComponent
                            case "String":
                                return stringComponent
                            case "List|Key":
                                return keyComponent
                            case "Enum":
                                return enumComponent
                            default:
                                return null
                            }
                        }

                        Component {
                            id: booleanComponent
                            D.Switch {
                                checked: optionDelegate.optValue === "True"
                                onCheckedChanged: {
                                    dccData.fcitx5ConfigProxy.setValue(
                                                root.name + "/" + optionDelegate.optName,
                                                checked ? "True" : "False")
                                }
                            }
                        }

                        Component {
                            id: integerComponent
                            D.SpinBox {
                                editable: true
                                width: 75
                                implicitWidth: 75
                                from: modelData.intMin !== undefined ? modelData.intMin : 0
                                to: modelData.intMax !== undefined ? modelData.intMax : 9999
                                value: parseInt(optionDelegate.optValue)
                                onValueChanged: {
                                    dccData.fcitx5ConfigProxy.setValue(
                                                root.name + "/" + optionDelegate.optName,
                                                value.toString())
                                }
                            }
                        }

                        Component {
                            id: stringComponent
                            D.TextField {
                                text: optionDelegate.optValue
                                onTextChanged: {
                                    dccData.fcitx5ConfigProxy.setValue(
                                                root.name + "/" + optionDelegate.optName,
                                                text)
                                }
                            }
                        }

                        Component {
                            id: keyComponent
                            KeySequenceDisplay {
                                placeholderText: qsTr("Please enter a new shortcut")
                                keys: optionDelegate.optValue
                                background.visible: false
                                onFocusChanged: {
                                    if (!focus) {
                                        if (keys.length > 0) {
                                            dccData.fcitx5ConfigProxy.setValue(
                                                        root.name + "/" + optionDelegate.optName + "/0",
                                                        keys, true)
                                        } else if (root.keyName != optionDelegate.optName) {
                                            keys = optionDelegate.optValue
                                        }
                                    }
                                }
                                onKeysChanged: {
                                    root.keyName = optionDelegate.optName
                                }

                                Connections {
                                    target: root
                                    function onKeyNameChanged() {
                                        if (root.keyName != optionDelegate.optName) {
                                            focus = false
                                        }
                                    }
                                }
                            }
                        }

                        Component {
                            id: enumComponent
                            D.ComboBox {
                                model: modelData.propertiesI18n
                                flat: true
                                currentIndex: modelData.properties.indexOf(
                                                  optionDelegate.optValue) ? modelData.properties.indexOf(
                                                                                 optionDelegate.optValue) : 0
                                onCurrentIndexChanged: {
                                    dccData.fcitx5ConfigProxy.setValue(
                                                root.name + "/" + optionDelegate.optName,
                                                modelData.properties[currentIndex])
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    Connections {
        target: dccData.fcitx5ConfigProxy
        function onRequestConfigFinished() {
            var newOptions = dccData.fcitx5ConfigProxy.globalConfigOptions(root.name)
            configValues = buildValuesMap(newOptions)

            if (!dccData.fcitx5ConfigProxy.saveTriggered) {
                configOptions = newOptions
                keyName = ""
            }
        }
    }

    Component.onCompleted: {
        var opts = dccData.fcitx5ConfigProxy.globalConfigOptions(root.name)
        configValues = buildValuesMap(opts)
        configOptions = opts
        loading = false
    }
}
