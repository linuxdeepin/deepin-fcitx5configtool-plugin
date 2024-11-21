// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import org.deepin.dtk 1.0 as D
import org.deepin.dcc 1.0

Rectangle {
    id: root

    color: "transparent"
    implicitHeight: layoutView.height
    Layout.fillWidth: true

    ColumnLayout {
        id: layoutView
        width: parent.width
        clip: true
        spacing: 0

        Repeater {
            id: repeater
            model: dccData.imlistModel()
            delegate: D.ItemDelegate {
                Layout.fillWidth: true
                visible: true
                cascadeSelected: true
                checkable: false
                contentFlow: true
                hoverEnabled: true
                corners: getCornersForBackground(index,
                                                 dccData.imlistModel().count())

                content: RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    DccLabel {
                        Layout.fillWidth: true
                        text: model.name
                    }

                    D.ToolButton {
                        id: imManageButton
                        icon.name: "dcc_immanage"
                        onClicked: imMenu.popup()

                        D.Menu {
                            id: imMenu

                            D.MenuItem {
                                text: qsTr("Move Up")
                                enabled: dccData.imlistModel().canMoveUp(index)
                                onTriggered: dccData.imlistModel().moveItem(
                                                 index, index - 1)
                            }

                            D.MenuItem {
                                text: qsTr("Move Down")
                                enabled: dccData.imlistModel(
                                             ).canMoveDown(index)
                                onTriggered: dccData.imlistModel().moveItem(
                                                 index, index + 1)
                            }

                            D.MenuItem {
                                text: qsTr("Settings")
                                onTriggered: dccData.showIMSettingsDialog(index)
                            }

                            D.MenuSeparator {}

                            D.MenuItem {
                                text: qsTr("Remove")
                                enabled: dccData.imlistModel().canRemove()
                                onTriggered: dccData.imlistModel(
                                                 ).removeItem(index)
                            }
                        }
                    }
                }
                background: DccItemBackground {
                    separatorVisible: true
                    backgroundType: DccObject.Normal | DccObject.Hover
                }
            }
        }
    }
}
