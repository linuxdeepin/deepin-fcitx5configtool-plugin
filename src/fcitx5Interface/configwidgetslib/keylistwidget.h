// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYLISTWIDGET_H_
#define KEYLISTWIDGET_H_

#include <QWidget>
#include <fcitx-utils/key.h>

class QToolButton;
class QBoxLayout;


class KeyListWidget : public QWidget {
    Q_OBJECT
public:
    explicit KeyListWidget(QWidget *parent = 0);

    QList<fcitx::Key> keys() const;
    void setKeys(const QList<fcitx::Key> &keys);
    void setAllowModifierLess(bool);
    void setAllowModifierOnly(bool);

signals:
    void keyChanged();

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    void addKey(fcitx::Key key = fcitx::Key());
    bool removeKeyAt(int idx);
    bool showRemoveButton() const;

    QToolButton *addButton_;
    QBoxLayout *keysLayout_;
    bool modifierLess_ = false;
    bool modifierOnly_ = false;
};


#endif // KEYLISTWIDGET_H_
