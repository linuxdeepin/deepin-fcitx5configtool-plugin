// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ADDIM_LAYOUTWIDGET_H
#define ADDIM_LAYOUTWIDGET_H

#include <DFrame>

struct xkb_context;

class LayoutWidget : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT

public:
    LayoutWidget(QWidget *parent = 0);
    ~LayoutWidget();

    void setKeyboardLayout(const QString &layout, const QString &variant = QString());

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    xkb_context *m_ctx;

    std::string m_layout;
    std::string m_variant;

    void paintLayout(QPainter &painter);
};

#endif
