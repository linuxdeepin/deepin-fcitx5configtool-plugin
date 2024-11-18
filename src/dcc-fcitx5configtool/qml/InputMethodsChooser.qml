import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Window
import QtQuick.Layouts 1.15

import org.deepin.dtk 1.0 as D
import org.deepin.dcc.fcitx5configtool 1.0

Loader {
    id: loader
    active: false
    //    required property var viewModel
    signal selected(string data)

    sourceComponent: D.DialogWindow {
        id: imDialog
        width: 420
        height: 550
        icon: "preferences-system"
        modality: Qt.WindowModal
        ColumnLayout {
            spacing: 10
            width: parent.width
            Label {
                Layout.alignment: Qt.AlignHCenter
                font.bold: true
                text: qsTr("Add input method")
            }

            D.SearchEdit {
                id: searchEdit
                Layout.fillWidth: true
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                placeholder: qsTr("Search")
                onTextChanged: {

                    //  viewModel.setFilterWildcard(text);
                }
                onEditingFinished: {

                    // viewModel.setFilterWildcard(text);
                }
            }

            ListView {
                id: itemsView
                property string checkedIM
                Layout.fillWidth: true
                height: 330
                clip: true
                // model: loader.viewModel
                ButtonGroup {
                    id: langGroup
                }

                section.property: "languageCode"
                section.criteria: ViewSection.FullString
                section.delegate: Rectangle {
                    width: ListView.view.width
                    height: childrenRect.height

                    required property string section

                    Text {
                        text: parent.section
                    }
                }

                delegate: D.CheckDelegate {
                    id: checkDelegate
                    implicitWidth: itemsView.width
                    text: model.display
                    hoverEnabled: true
                    ButtonGroup.group: langGroup
                    onCheckedChanged: {
                        if (checked) {
                            itemsView.checkedIM = model.display
                        }
                    }
                }
            }

            D.Button {
                id: addImBtn
                text: qsTr("Find more in App Store")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.rightMargin: 10
                background: null
                textColor: D.Palette {
                    normal: D.DTK.makeColor(D.Color.Highlight)
                }

                onClicked: {
                    console.log("Find more in App Store button clicked")
                    dccData.openDeepinAppStore()
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 10
                Button {
                    Layout.bottomMargin: 10
                    text: qsTr("Cancel")
                    onClicked: {
                        imDialog.close()
                    }
                }
                Button {
                    Layout.bottomMargin: 10
                    text: qsTr("Add")
                    enabled: itemsView.checkedIM.length > 0
                    onClicked: {
                        selected(itemsView.checkedIM)
                        imDialog.close()
                    }
                }
            }
        }
        onClosing: {
            loader.active = false
            // viewModel.setFilterWildcard("");
        }
    }

    onLoaded: {
        item.show()
    }
}
