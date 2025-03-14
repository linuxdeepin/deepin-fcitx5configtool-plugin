// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15
import QtQml.Models 2.15

import org.deepin.dtk 1.0 as D
import org.deepin.dcc 1.0

Rectangle {
    id: root

    color: "transparent"
    implicitHeight: listView.height
    Layout.fillWidth: true

    DelegateModel {
        id: visualModel
        model: dccData.imlistModel()
        delegate: dragDelegate
    }

    Component {
        id: dragDelegate
        D.ItemDelegate {
            id: delegateItem
            width: listView.width
            height: content.height
            cascadeSelected: true
            checkable: false
            contentFlow: true
            hoverEnabled: true
            corners: getCornersForBackground(DelegateModel.itemsIndex,
                                             dccData.imlistModel().count())

            background: DccItemBackground {
                separatorVisible: true
                backgroundType: DccObject.Normal | DccObject.Hover
            }

            MouseArea {
                id: dragArea
                anchors.fill: parent
                drag.target: content
                drag.axis: Drag.YAxis
                drag.smoothed: false

                property int fromIndex: -1 // 记录起始位置
                property int toIndex: -1 // 记录目标位置
                property bool isDragging: drag.active // 通过绑定来跟踪拖拽状态

                // 开始拖拽时记录起始位置
                onPressed: {
                    fromIndex = DelegateModel.itemsIndex
                }

                // 释放时保存数据到真正的模型中
                onReleased: {
                    // 只有在实际发生拖拽且位置发生变化时才保存
                    console.log("onReleased isDragging:", isDragging,
                                "fromIndex:", fromIndex, "toIndex:", toIndex)
                    if (isDragging && fromIndex >= 0 && toIndex >= 0
                            && fromIndex !== toIndex) {
                        dccData.imlistModel().moveItem(fromIndex, toIndex)
                    }
                    // 重置状态
                    fromIndex = -1
                    toIndex = -1
                }

                DropArea {
                    anchors {
                        fill: parent
                        margins: 10
                    }
                    onEntered: function (drag) {
                        let currentIndex = dragArea.DelegateModel.itemsIndex
                        // 更新源MouseArea的toIndex，而不是当前MouseArea的toIndex
                        drag.source.toIndex = currentIndex
                        console.log("==> enter",
                                    drag.source.DelegateModel.itemsIndex, "to",
                                    currentIndex)
                        visualModel.items.move(
                                    drag.source.DelegateModel.itemsIndex,
                                    currentIndex)
                    }
                }
                Rectangle {
                    id: content
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        verticalCenter: parent.verticalCenter
                    }
                    width: dragArea.width
                    height: rowLayout.height
                    color: dragArea.drag.active ? Qt.rgba(0, 0, 0,
                                                          0.1) : "transparent"

                    Behavior on color {
                        ColorAnimation {
                            duration: 100
                        }
                    }

                    Drag.active: dragArea.drag.active
                    Drag.source: dragArea
                    Drag.hotSpot.x: width / 2
                    Drag.hotSpot.y: height / 2

                    states: State {
                        when: content.Drag.active
                        ParentChange {
                            target: content
                            parent: root
                        }
                        AnchorChanges {
                            target: content
                            anchors {
                                horizontalCenter: undefined
                                verticalCenter: undefined
                            }
                        }
                    }

                    RowLayout {
                        id: rowLayout
                        width: parent.width
                        Layout.fillHeight: true
                        spacing: 10
                        anchors {
                            left: parent.left
                            right: parent.right
                            leftMargin: 10
                            rightMargin: 10
                        }

                        DccLabel {
                            Layout.fillWidth: true
                            text: model.name
                        }

                        D.ToolButton {
                            id: imManageButton
                            icon.name: "dcc_input_option"
                            onClicked: imMenu.popup()

                            D.Menu {
                                id: imMenu

                                D.MenuItem {
                                    text: qsTr("Move Up")
                                    enabled: dccData.imlistModel(
                                                 ).canMoveUp(index)
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
                                    onTriggered: dccData.showIMSettingsDialog(
                                                     index)
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
                }
            }
        }
    }

    ListView {
        id: listView
        width: parent.width
        height: contentHeight
        model: visualModel
        spacing: 4
        interactive: true
        clip: true
        cacheBuffer: 50

        moveDisplaced: Transition {
            NumberAnimation {
                properties: "x,y"
                duration: 200
            }
        }
    }
}
