/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SETTINGSHEAD_H
#define SETTINGSHEAD_H

#include "settingsitem.h"
#include "titlelabel.h"

#include <QPushButton>
#include <DIconButton>

DWIDGET_BEGIN_NAMESPACE
class DCommandLinkButton;
DWIDGET_END_NAMESPACE
class ConfigSettings;
namespace dcc_fcitx_configtool {
namespace widgets {

class FcitxNormalLabel;

class FcitxSettingsHead : public FcitxSettingsItem
{
    Q_OBJECT

public:
    enum State {
        Edit,
        Cancel
    };

public:
    explicit FcitxSettingsHead(bool isEdit = false, QFrame *parent = nullptr);
    ~FcitxSettingsHead();

    void setTitle(const QString &title);
    void setDeleteEnable(bool state = true);
    void hideDeleteEnable();
    void hideAddEnable();
    void setAddEnable();
    FcitxTitleLabel* getTitleLabel();

Q_SIGNALS:
    void editChanged(bool edit);
    void deleteBtnClicked();
    void addBtnClicked();
private:
    FcitxTitleLabel *m_title;
    DTK_WIDGET_NAMESPACE::DIconButton *m_deleteBtn;
    DTK_WIDGET_NAMESPACE::DIconButton *m_addBtn;
    bool m_editVisible;
    State m_state;
    ConfigSettings *m_setting;
};

} // namespace widgets
} // namespace dcc_fcitx_configtool

#endif // SETTINGSHEAD_H
