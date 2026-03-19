// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.deepin.dtk 1.0 as D

Label {
    Layout.fillWidth: true
    property D.Palette textColor: D.Palette {
        normal: Qt.rgba(0, 0, 0, 0.9)
        normalDark: Qt.rgba(1, 1, 1, 0.9)
    }
    font {
        family: D.DTK.fontManager.t5.family
        pixelSize: D.DTK.fontManager.t5.pixelSize
        weight: 500
    }
    color: D.ColorSelector.textColor
}
