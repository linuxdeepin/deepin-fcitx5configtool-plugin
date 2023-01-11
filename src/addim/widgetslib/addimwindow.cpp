/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "glo.h"
#include "imelog.h"
#include "addimwindow.h"

#include <QKeyEvent>
#include <QPushButton>
#include <fcitx-utils/i18n.h>
#if DTK_VERSION_MAJOR == 5
#include <DTitlebar>
#endif

namespace fcitx {
namespace addim {

AddIMWindow::AddIMWindow(DBusProvider* dbus, IMConfig* config, DDialog* parent)
    : DDialog(parent)
    , m_dbus(dbus)
    , m_impage(new IMPage(m_dbus, config, this))
{
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "====>\n");
    m_config = config;

    /** add titleBar */

    setIcon(QIcon(":/img/title_img.png"));    
    setupUi(this);
    //浏览使用语言并添加输入法
    setWindowTitle(tr("Select your language and add input methods"));

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowMinimizeButtonHint);
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "before, connect.\n");
    connect(m_impage, &IMPage::changed, this, [this]() {
        emit changed(true);
    });
    pageWidget->addWidget(m_impage);

    connect(this, &AddIMWindow::changed, this, &AddIMWindow::handleChanged);
    connect(m_impage, &IMPage::closeAddIMWindow, this, &AddIMWindow::closeWindow);

    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "before, load.\n");
    load();
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "after, load.\n");
}

void AddIMWindow::handleChanged(bool changed) {
}

void AddIMWindow::closeWindow()
{
	this->close();
}

void AddIMWindow::load() {
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "====>\n");
    m_impage->load();
    emit changed(false);
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "<====\n");
}

void AddIMWindow::keyPressEvent(QKeyEvent *event) {
    DDialog::keyPressEvent(event);
    if (!event->isAccepted() && event->matches(QKeySequence::Cancel)) {
        qApp->quit();
    }
}

void AddIMWindow::closeEvent(QCloseEvent* event)
{
    DDialog::closeEvent(event);
    setUseIMLanguageCount(0);
    setCurrentIMViewIndex(-1);
}

}
}
