// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import org.deepin.dcc 1.0
import org.deepin.dtk 1.0 as D

DccObject {
    // Manage input methods
    DccObject {
        name: "ManageInputMethods"
        parentName: "Manage Input Methods"
        weight: 100
        pageType: DccObject.Item
        onParentItemChanged: {
            if (parentItem) {
                parentItem.bottomPadding = 0
            }
        }
        page: DccGroupView {
            spacing: 0
            isGroup: false
        }
        ManageInputMethodsModule {}
    }

    // Shortcuts
    DccObject {
        name: "Shortcuts"
        parentName: "Manage Input Methods"
        weight: 200
        pageType: DccObject.Item
        onParentItemChanged: {
            if (parentItem) {
                parentItem.topPadding = 5
                parentItem.bottomPadding = 0
            }
        }
        page: DccGroupView {
            spacing: 0
            isGroup: false
        }
        ShortcutsModule {}
    }

    // Advanced Settings
    DccObject {
        name: "AdvancedSettings"
        parentName: "Manage Input Methods"
        weight: 300
        pageType: DccObject.Item
        onParentItemChanged: {
            if (parentItem) {
                parentItem.topPadding = 5
            }
        }
        page: DccGroupView {
            spacing: 0
            isGroup: false
        }
        AdvancedSettingsModule {}
    }
}
