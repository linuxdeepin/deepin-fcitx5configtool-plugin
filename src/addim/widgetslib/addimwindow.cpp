// SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#include "addimwindow.h"

#include "glo.h"

#include <fcitx-utils/i18n.h>

#include <QDebug>
#include <QKeyEvent>
#include <QPushButton>

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
    qInfo("====>");
    m_config = config;

    /** add titleBar */

    setIcon(QIcon(":/img/title_img.png"));    
    setupUi(this);
    //浏览使用语言并添加输入法
    setWindowTitle(tr("Select your language and add input methods"));

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowMinimizeButtonHint);
    qInfo("before, connect.");
    connect(m_impage, &IMPage::changed, this, [this]() {
        emit changed(true);
    });
    pageWidget->addWidget(m_impage);

    connect(this, &AddIMWindow::changed, this, &AddIMWindow::handleChanged);
    connect(m_impage, &IMPage::closeAddIMWindow, this, &AddIMWindow::closeWindow);

    qInfo("before, load.");
    load();
    qInfo("after, load.");
}

void AddIMWindow::handleChanged(bool changed) {
}

void AddIMWindow::closeWindow()
{
	this->close();
}

void AddIMWindow::load() {
    qInfo("====>");
    m_impage->load();
    emit changed(false);
    qInfo("<====");
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
