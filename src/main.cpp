/*
* Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
*
* Author:     chenshijie <chenshijie@uniontech.com>
*
* Maintainer: chenshijie <chenshijie@uniontech.com>
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

#include "publisher/publisherdef.h"
#include "window/imwindow.h"
#include <DApplication>
#include <DWidgetUtil>
#include <DApplicationSettings>
#include <DMainWindow>
#include <QVBoxLayout>
#include <thread>
#include <QObject>
#include <QPushButton>
#include "widgets/settingsgroup.h"
#include "widgets/settingsitem.h"
DWIDGET_USE_NAMESPACE
using namespace std;
using namespace dcc_fcitx_configtool::widgets;

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    DApplication a(argc, argv);
    a.setOrganizationName("deepin");
    a.setApplicationName("dcc-fcitx5configtool-plugin");
    a.setApplicationVersion("1.0");
    a.setProductIcon(QIcon());
    a.setProductName("dcc-fcitx5configtool-plugin");
    a.setApplicationDescription("This is a dtk dcc-fcitx5configtool-plugin");
    a.loadTranslator();
    a.setApplicationDisplayName("dcc-fcitx5configtool-plugin");
    DMainWindow m;
    IMWindow w(&m);
    m.resize(800, 600);
    m.setCentralWidget(&w);
    //w.setGeometry(m.geometry());
    m.show();
    //w.show();
    Dtk::Widget::moveToCenter(&w);

    return a.exec();
}
