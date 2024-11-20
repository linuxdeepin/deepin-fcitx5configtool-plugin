// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
import Qt.labs.qmlmodels 1.2

import org.deepin.dtk 1.0 as D

import org.deepin.dcc 1.0
import org.deepin.dcc.fcitx5configtool 1.0

DccObject {

    // title
    DccObject {
        name: "AddonsListTitle"
        parentName: "AddonsPage"
        displayName: qsTr("Add-ons")
        weight: 110
        hasBackground: false
        pageType: DccObject.Item
        page: RowLayout {
            width: parent.width
            Label {
                Layout.leftMargin: 10
                Layout.fillWidth: true
                font: D.DTK.fontManager.t4
                text: dccObj.displayName
            }
        }
    }

    DetailAddonsItem {}
}

