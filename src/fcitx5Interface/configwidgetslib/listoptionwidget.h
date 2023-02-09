// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LISTOPTIONWIDGET_H_
#define LISTOPTIONWIDGET_H_

#include "optionwidget.h"
#include "ui_listoptionwidget.h"



class ListOptionWidgetModel;

class ListOptionWidget : public OptionWidget, public Ui::ListOptionWidget {
    Q_OBJECT
public:
    ListOptionWidget(const fcitx::FcitxQtConfigOption &option, const QString &path,
                     QWidget *parent);

    void readValueFrom(const QVariantMap &map) override;
    void writeValueTo(QVariantMap &map) override;
    void restoreToDefault() override;

    const auto &subOption() { return subOption_; }

private:
    void updateButton();
    ListOptionWidgetModel *model_;
    fcitx::FcitxQtConfigOption subOption_;
    QVariantMap defaultValue_;
};


#endif // LISTOPTIONWIDGET_H_
