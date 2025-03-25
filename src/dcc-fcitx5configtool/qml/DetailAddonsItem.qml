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
    property var globalAddons: []

    Component.onCompleted: {
        dccData.fcitx5AddonsProxy.onRequestAddonsFinished.connect(() => {
                                                                      globalAddons = dccData.fcitx5AddonsProxy.globalAddons()
                                                                  })
        globalAddons = dccData.fcitx5AddonsProxy.globalAddons()
        loading = false
    }

    Loader {
        active: !loading
        asynchronous: true
        sourceComponent: DccRepeater {
            model: globalAddons
            delegate: DccObject {
                property bool expanded: true
                parentName: "AddonsPage"
                backgroundType: DccObject.Normal | DccObject.Hover
                weight: 120 + index
                pageType: DccObject.Item
                property bool isHovered: false
                onParentItemChanged: {
                    if (parentItem) {
                        parentItem.leftPadding = 4
                    }
                }
                page: RowLayout {
                    height: 50
                    spacing: 0
                    CheckBox {
                        Layout.alignment: Qt.AlignLeft
                        checked: modelData.enabled
                        onClicked: {
                            dccData.fcitx5AddonsProxy.setEnableAddon(
                                        modelData.uniqueName, checked)
                        }
                    }
                    ColumnLayout {
                        Layout.alignment: Qt.AlignLeft
                        spacing: 0
                        DccLabel {
                            Layout.fillHeight: false
                            font: D.DTK.fontManager.t6
                            text: modelData.name
                        }
                        DccLabel {
                            Layout.fillHeight: false
                            font: D.DTK.fontManager.t8
                            text: modelData.comment
                            opacity: 0.5
                            visible: modelData.comment !== ""
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    D.ToolButton {
                        id: imManageButton
                        Layout.alignment: Qt.AlignRight
                        icon.name: "dcc_input_method_settings"
                        visible: modelData.configurable && isHovered
                        enabled: modelData.enabled
                        onClicked: {
                            dccData.showAddonSettingsDialog(
                                        modelData.uniqueName,
                                        modelData.name)
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: isHovered = true
                        onExited: isHovered = false
                        acceptedButtons: Qt.NoButton
                    }
                }
            }
        }
    }
}
