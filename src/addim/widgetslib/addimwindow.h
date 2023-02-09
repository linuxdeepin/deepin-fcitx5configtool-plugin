/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef ADDIM_WINDOW_H
#define ADDIM_WINDOW_H

#include "dbusprovider.h"
#include "impage.h"
#include "ui_addimwindow.h"
#include <QAbstractButton>

#if DTK_VERSION_MAJOR == 5
#include <DDialog>
#endif

DWIDGET_USE_NAMESPACE

namespace fcitx {
namespace addim {

class AddIMWindow : public DDialog, public Ui::MainWindow {
    Q_OBJECT

public:
    explicit AddIMWindow(DBusProvider* dbus, IMConfig* config, DDialog* parent = nullptr);

    void load();

signals:
    void changed(bool state);

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void closeEvent(QCloseEvent* event) override;

protected:
    IMConfig* m_config;

private:
    void handleChanged(bool state);
    void closeWindow();

private:
    DBusProvider *m_dbus;
    IMPage *m_impage;
};
}
}

#endif
