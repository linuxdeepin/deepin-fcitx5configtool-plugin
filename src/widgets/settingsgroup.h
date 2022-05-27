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

#ifndef SETTINGSGROUP_H
#define SETTINGSGROUP_H

#include <DBackgroundGroup>
#include <QFrame>
#include <QTimer>
#include <QDateTime>
#include "publisher/publisherdef.h"

#include "translucentframe.h"

class QVBoxLayout;

DWIDGET_USE_NAMESPACE

namespace dcc_fcitx_configtool {
namespace widgets {

class FcitxSettingsItem;
class FcitxSettingsHeaderItem;
class FcitxSettingsHead;

class FcitxSettingsGroup : public FcitxTranslucentFrame
{
    Q_OBJECT

public:
    enum BackgroundStyle {
        ItemBackground = 0,
        GroupBackground,
        NoneBackground,
        NonSwitchMode
    };

    explicit FcitxSettingsGroup(QFrame *parent = nullptr, BackgroundStyle bgStyle = ItemBackground);
    explicit FcitxSettingsGroup(const QString &title, QFrame *parent = nullptr);
    ~FcitxSettingsGroup();

    FcitxSettingsHeaderItem *headerItem() { return m_headerItem; }
    void setHeaderVisible(const bool visible);

    FcitxSettingsItem *getItem(int index);
    void insertWidget(QWidget *widget);
    void insertItem(const int index, FcitxSettingsItem *item);
    void appendItem(FcitxSettingsItem *item);
    void appendItem(FcitxSettingsItem *item, BackgroundStyle bgStyle);
    void removeItem(FcitxSettingsItem *item);
    int indexOf(FcitxSettingsItem *item);
    void moveItem(FcitxSettingsItem *item, const int index);
    void setSpacing(const int spaceing);
    bool isPressed() {return m_isPressed;}
    void setSwitchAble(bool b){m_switchAble = b;}
    int itemCount() const;
    void clear();
    QVBoxLayout *getLayout() const { return m_layout; }
    void switchItem(int start, int end);
    void setVerticalPolicy();
    int selectIndex() {return m_selectIndex;}
protected:
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void focusOutEvent(QFocusEvent *e) override;
private slots:
    void onSelectItem(FcitxSettingsItem *item, bool selected);

signals:
    void switchPosition(FcitxQtInputMethodItem* item, int end);

private:
    BackgroundStyle m_bgStyle {ItemBackground};
    QVBoxLayout *m_layout;
    FcitxSettingsHeaderItem *m_headerItem;
    FcitxSettingsItem* m_lastItem {nullptr};
    DTK_WIDGET_NAMESPACE::DBackgroundGroup *m_bggroup {nullptr};
    bool m_isPressed {false};
    int m_selectIndex {0};
    bool m_switchAble {false};
    int m_lastYPosition{0};
    QDateTime m_time;
};

} // namespace widgets
} // namespace dcc_fcitx_configtool

#endif // SETTINGSGROUP_H
