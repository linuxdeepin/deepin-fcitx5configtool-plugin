// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
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
    property var keyName: ""

    DccObject {
        name: root.displayName
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
    }

    DccObject {
        id: headerItem
        property bool expanded: false
        parentName: root.displayName
        displayName: root.displayName
        weight: root.weight
        pageType: DccObject.Item
        backgroundType: DccObject.Normal | DccObject.Hover
        page: RowLayout {
            id: headerRow
            height: 40

            MouseArea {
                anchors.fill: parent
                onClicked: headerItem.expanded = !headerItem.expanded
            }

            Label {
                Layout.leftMargin: 10
                Layout.alignment: Qt.AlignVCenter
                font: D.DTK.fontManager.t6
                text: dccObj.displayName
            }
            D.ToolButton {
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                icon.width: 12
                icon.height: 12
                icon.name: headerItem.expanded ? "go-down" : "go-next"
                onClicked: headerItem.expanded = !headerItem.expanded
            }
        }
    }

    Component.onCompleted: {
        dccData.fcitx5ConfigProxy.onRequestConfigFinished.connect(() => {
                                                                      configOptions = []
                                                                      configOptions = dccData.fcitx5ConfigProxy.globalConfigOptions(
                                                                          root.name)
                                                                      keyName = ""
                                                                  })
        configOptions = dccData.fcitx5ConfigProxy.globalConfigOptions(root.name)
        loading = false
    }

    Loader {
        active: !loading
        asynchronous: true
        sourceComponent: DccRepeater {
            model: configOptions
            delegate: Component {
                DccObject {
                    parentName: root.displayName
                    displayName: modelData.description
                    weight: root.weight + index + 1
                    pageType: DccObject.Editor
                    visible: headerItem.expanded
                    backgroundType: DccObject.Normal | DccObject.Hover
                    page: Loader {
                        height: 40
                        sourceComponent: {
                            switch (modelData.type) {
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
                                checked: modelData.value === "True"
                                onCheckedChanged: {
                                    dccData.fcitx5ConfigProxy.setValue(
                                                root.name + "/" + modelData.name,
                                                checked ? "True" : "False")
                                }
                            }
                        }

                        Component {
                            id: integerComponent
                            D.SpinBox {
                                width: 55
                                implicitWidth: 55
                                value: parseInt(modelData.value)
                                onValueChanged: {
                                    dccData.fcitx5ConfigProxy.setValue(
                                                root.name + "/" + modelData.name,
                                                value.toString())
                                }
                            }
                        }

                        Component {
                            id: stringComponent
                            D.TextField {
                                text: modelData.value
                                onTextChanged: {
                                    dccData.fcitx5ConfigProxy.setValue(
                                                root.name + "/" + modelData.name,
                                                text)
                                }
                            }
                        }

                        Component {
                            id: keyComponent
                            KeySequenceDisplay {
                                placeholderText: qsTr("Please enter a new shortcut")
                                keys: modelData.value
                                background.visible: false
                                onFocusChanged: {
                                    if (!focus) {
                                        if (keys.length > 0) {
                                            dccData.fcitx5ConfigProxy.setValue(
                                                        root.name + "/" + modelData.name + "/0",
                                                        keys, true)
                                        } else if (keyName != modelData.name) {
                                            keys = modelData.value
                                        }
                                    }
                                }
                                onKeysChanged: {
                                    root.keyName = modelData.name
                                }

                                Connections {
                                    target: root
                                    function onKeyNameChanged() {
                                        if (keyName != modelData.name) {
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
                                                  modelData.value) ? modelData.properties.indexOf(
                                                                         modelData.value) : 0
                                onCurrentIndexChanged: {
                                    dccData.fcitx5ConfigProxy.setValue(
                                                root.name + "/" + modelData.name,
                                                modelData.properties[currentIndex])
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
