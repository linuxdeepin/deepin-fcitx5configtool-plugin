/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "glo.h"
#include "ime_log.h"
#include "addim_window.h"

#include <QKeyEvent>
#include <QPushButton>
#include <fcitx-utils/i18n.h>
#if DTK_VERSION_MAJOR == 5
#include <DTitlebar>
#endif

namespace fcitx {
namespace addim {

AddIMWindow::AddIMWindow(DBusProvider* dbus, DDialog* parent)
    : DDialog(parent)
    , m_dbus(dbus)
    , m_impage(new IMPage(m_dbus, this))
{
    osa_log_info(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "====>\n");
    setAttribute(Qt::WA_TranslucentBackground, false);
    /** add titleBar */

    setIcon(QIcon::fromTheme("dcc_chineseime"));
    setWindowTitle("浏览使用语言并添加输入法");

    setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowMinimizeButtonHint);

    osa_log_info(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "before, connect.\n");
    connect(m_impage, &IMPage::changed, this, [this]() {
        emit changed(true);
    });
    pageWidget->addWidget(m_impage);

    connect(this, &AddIMWindow::changed, this, &AddIMWindow::handleChanged);
    connect(m_impage, &IMPage::closeAddIMWindow, this, &AddIMWindow::closeWindow);

    osa_log_info(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "before, load.\n");
    initCategroyLanguageMap();
    load();
    osa_log_info(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "after, load.\n");
}

void AddIMWindow::handleChanged(bool changed) {
}

void AddIMWindow::closeWindow()
{
	this->close();
}

void AddIMWindow::load() {
    osa_log_info(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "====>\n");
    m_impage->load();
    emit changed(false);
    osa_log_info(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "<====\n");
}

void AddIMWindow::keyPressEvent(QKeyEvent *event) {
    DDialog::keyPressEvent(event);
    if (!event->isAccepted() && event->matches(QKeySequence::Cancel)) {
        qApp->quit();
    }
}

}
}
