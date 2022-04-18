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

#ifndef CONTENTWIDGET_H
#define CONTENTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <DImageButton>

DWIDGET_USE_NAMESPACE

class QScrollArea;
class QPropertyAnimation;
class QPushButton;

namespace dcc_fcitx_configtool {

namespace widgets {
class BackButton;
}
} // namespace dcc_fcitx_configtool

namespace dcc_fcitx_configtool {
namespace widgets {
class FcitxContentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FcitxContentWidget(QWidget *parent = 0);
    ~FcitxContentWidget();

    QWidget *content() const { return m_content; }
    QWidget *setContent(QWidget *const w);
    void scrollTo(int dy);

protected:
    void resizeEvent(QResizeEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
protected:
    QScrollArea *m_contentArea {nullptr};
    QWidget *m_content {nullptr};
};

} // namespace widgets
} // namespace dcc_fcitx_configtool

#endif // CONTENTWIDGET_H
