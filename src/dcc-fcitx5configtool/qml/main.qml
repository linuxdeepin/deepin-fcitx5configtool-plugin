// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.3

import org.deepin.dcc 1.0

DccObject {
    id: root

    DccObject {
        name: "Fcitx5ManageInputMethods"
        parentName: "Fcitx5configtool"
        weight: 10
        pageType: DccObject.Item
        page: DccGroupView {
            spacing: 5
            isGroup: false
            height: implicitHeight + 20
        }
        ManageInputMethods {}
    }
}
