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
        name: "ManageInputMethodsTitle"
        parentName: "Manage Input Methods"
        displayName: qsTr("Manage Input Methods")
        weight: 110
        pageType: DccObject.Item
        page: RowLayout {
            width: parent.width
            Label {
                Layout.leftMargin: 10
                Layout.fillWidth: true
                font: D.DTK.fontManager.t4
                text: dccObj.displayName
            }

            D.Button {
                id: addImBtn
                text: qsTr("Add input method")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.rightMargin: 10
                background: null
                textColor: D.Palette {
                    normal: D.DTK.makeColor(D.Color.Highlight)
                }
                InputMethodsChooser {
                    id: imsChooser
                    viewModel: dccData.imProxyModel()
                    onSelected: function (index) {
                        console.log("selected im index", index)
                        dccData.addIM(index)
                    }
                }

                onClicked: {
                    console.log("Add input method button clicked")
                    imsChooser.active = true
                }
            }
        }
    }

    // list
    DccObject {
        name: "currentIMList"
        parentName: "Manage Input Methods"
        weight: 120
        backgroundType: DccObject.Normal
        pageType: DccObject.Item
        page: IMList {}
    }
}
