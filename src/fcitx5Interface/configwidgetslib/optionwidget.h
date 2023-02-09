// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONWIDGET_H_
#define OPTIONWIDGET_H_

#include <QWidget>
#include <fcitxqtdbustypes.h>

class QFormLayout;

class OptionWidget : public QWidget {
    Q_OBJECT
public:
    OptionWidget(const QString &path, QWidget *parent)
        : QWidget(parent), path_(path) {}

    static OptionWidget *addWidget(QFormLayout *layout,
                                   const fcitx::FcitxQtConfigOption &option,
                                   const QString &path, QWidget *parent);

    static bool execOptionDialog(QWidget *parent,
                                 const fcitx::FcitxQtConfigOption &option,
                                 QVariant &result);

    virtual void readValueFrom(const QVariantMap &map) = 0;
    virtual void writeValueTo(QVariantMap &map) = 0;
    virtual void restoreToDefault() = 0;
    virtual bool isValid() const { return true; }

    QString prettify(const fcitx::FcitxQtConfigOption &option, const QVariant &value);

    const QString &path() const { return path_; }
signals:
    void valueChanged();

private:
    QString path_;
};

#endif // OPTIONWIDGET_H_
