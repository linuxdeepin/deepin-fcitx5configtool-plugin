/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
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

#ifndef SETTINGSITEM_H
#define SETTINGSITEM_H

#include <DFrame>
#include <QLabel>
#include <QApplication>
#include <DStyle>
#include <QFrame>
#include <QVBoxLayout>
namespace dcc_fcitx_configtool {
namespace widgets {

class FcitxSettingsItem : public QFrame
{
public:
    enum itemPosition{
        firstItem = 0,
        lastItem = -1,
        onlyoneItem = -2,
        otherItem = 1
    };

    Q_OBJECT
    Q_PROPERTY(bool isErr READ isErr DESIGNABLE true SCRIPTABLE true)

public:
    explicit FcitxSettingsItem(QWidget *parent = nullptr);

    bool isErr() const;
    virtual void setIsErr(const bool err = true);

    void addBackground();
    bool isDraged() {return m_isDraged;}
    void setDraged(bool b) {m_isDraged = b;}
    void setPaint(bool b) {m_neetPaint = b;}

    bool isSelected() const;
    void setIsSelected(bool isSelected);

    bool isEntered() const;
    void setIsEntered(bool isEntered);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

protected:
    DTK_WIDGET_NAMESPACE::DFrame *m_bgGroup {nullptr};

private:
    int m_index {0};
    bool m_isDraged {false};
    bool m_isSelected {false};
    bool m_isEntered {false};
    bool m_neetPaint {false};
    bool m_isErr {false};
};

} // namespace widgets
} // namespace dcc

#endif // SETTINGSITEM_H
