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
            model: imListModel
            delegate: D.ItemDelegate {
                Layout.fillWidth: true
                visible: true
                leftPadding: 10
                rightPadding: 10
                cascadeSelected: true
                checkable: false
                contentFlow: true
                corners: getCornersForBackground(index, imListModel.count)

                content: RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    DccLabel {
                        Layout.fillWidth: true
                        text: model.title
                    }

                    D.ToolButton {
                        icon.name: "dcc_immanage"
                    }
                }
                background: DccItemBackground {
                    separatorVisible: true
                    highlightEnable: false
                }
            }
        }
    }

    // TODO(zhangs): read from config
    ListModel {
        id: imListModel

        ListElement {
            title: qsTr("AAAAAAAAA")
        }
        ListElement {
            title: qsTr("BBBBBBBBBB")
        }
        ListElement {
            title: qsTr("CCCCCCCC")
        }
        ListElement {
            title: qsTr("DDDDDDDDDD")
        }
    }
}
