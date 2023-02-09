// SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FONTBUTTON_H_
#define FONTBUTTON_H_

#include "ui_fontbutton.h"


class FontButton : public QWidget, public Ui::FontButton {
    Q_OBJECT
public:
    explicit FontButton(QWidget *parent = 0);
    virtual ~FontButton();
    const QFont &font();
    QString fontName();

public slots:
    void setFont(const QFont &font);
signals:
    void fontChanged(const QFont &font);
private slots:
    void selectFont();

private:
    QFont font_;
};


#endif // _KCM_FCITX_FONTBUTTON_H_
