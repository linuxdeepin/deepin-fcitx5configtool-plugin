// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

Window {
    id: control

    maximumWidth: Screen.desktopAvailableWidth
    maximumHeight: Screen.desktopAvailableHeight
    minimumWidth: DS.Style.dialogWindow.width
    minimumHeight: DS.Style.dialogWindow.height
    D.DWindow.enabled: true
    D.DWindow.enableSystemResize: true  // 启用系统调整大小
    D.DWindow.motifFunctions: D.WindowManagerHelper.FUNC_ALL  // 允许所有窗口功能，包括调整大小
    D.DWindow.wmWindowTypes: D.WindowManagerHelper.DialogType
    D.DWindow.enableBlurWindow: true
    flags: Qt.Dialog | Qt.WindowCloseButtonHint | Qt.WindowMinMaxButtonsHint  // 添加最大化最小化按钮
    D.ColorSelector.family: D.Palette.CrystalColor
    color: active ? D.DTK.palette.window : D.DTK.inactivePalette.window

    property alias header: titleBar.sourceComponent
    property string icon
    default property alias content: contentLoader.children
    property alias palette : content.palette

    Item {
        id: content
        anchors.fill: parent
        palette: control.active ? D.DTK.palette : D.DTK.inactivePalette
        
        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: 0

            Loader {
                id: titleBar
                z: D.DTK.TopOrder
                Layout.fillWidth: true
                sourceComponent: D.DialogTitleBar {
                    enableInWindowBlendBlur: true
                    icon.name: control.icon
                }
            }

            Item {
                id: contentLoader
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: DS.Style.dialogWindow.contentHMargin
                Layout.rightMargin: DS.Style.dialogWindow.contentHMargin
            }
        }
    }

    onClosing: function(close) {
        // close can't reset sub control's hovered state. pms Bug:168405
        // if we need to close, we can add closing handler to set `close.acceped = true`
        close.accepted = false
        hide()
    }
} 