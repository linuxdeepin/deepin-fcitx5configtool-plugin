/*
 * SPDX-FileCopyrightText: 2011~2011 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "config.h"
#include "keyboardlayoutwidget.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QMainWindow>
#include <QMessageBox>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>

int main(int argc, char *argv[]) {
    qDebug() << "Starting keyboard layout viewer application";
    qputenv("QT_QPA_PLATFORM", "xcb");

    QApplication app(argc, argv);
    qDebug() << "Qt application initialized";
    app.setApplicationName(QLatin1String("kbd-layout-viewer"));
    app.setApplicationVersion(QLatin1String(PROJECT_VERSION));

    qDebug() << "Checking X11 platform support";
    QNativeInterface::QX11Application *x11App = app.nativeInterface<QNativeInterface::QX11Application>();
    if (!x11App) {
        qCritical() << "X11 platform not supported, application will exit";
        QMessageBox msgBox(QMessageBox::Critical, _("Error"),
                           _("This program only works on X11."));
        msgBox.exec();
        return 1;
    }

    qDebug() << "Initializing command line parser";
    QCommandLineParser parser;
    parser.setApplicationDescription(_("A general keyboard layout viewer"));
    parser.addHelpOption();
    parser.addOptions(
        {{{"g", "group"}, _("Keyboard layout <group> (0-3)"), _("group")},
         {{"l", "layout"}, _("Keyboard <layout>"), _("layout")},
         {{"v", "variant"}, _("Keyboard layout <variant>"), _("variant")}});

    parser.process(app);
    qDebug() << "Command line arguments processed";

    int group = -1;
    QString variant, layout;
    if (parser.isSet("group")) {
        group = parser.value("group").toInt();
        qDebug() << "Command line group parameter:" << group;
    }
    if (parser.isSet("layout")) {
        layout = parser.value("layout");
        qDebug() << "Command line layout parameter:" << layout;
    }
    if (parser.isSet("variant")) {
        variant = parser.value("variant");
        qDebug() << "Command line variant parameter:" << variant;
    }

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    qDebug() << "Creating main application window";
    QMainWindow mainWindow;
    mainWindow.setWindowIcon(QIcon::fromTheme("input-keyboard"));
    mainWindow.setWindowTitle(_("Keyboard Layout viewer"));
    mainWindow.setMinimumSize(QSize(900, 400));
    fcitx::kcm::KeyboardLayoutWidget widget;
    qDebug() << "Setting keyboard layout parameters";
    if (group > 0 || layout.isNull()) {
        if (group < 0) {
            qDebug() << "Using default group 0";
            group = 0;
        }
        qDebug() << "Setting keyboard group to:" << group;
        widget.setGroup(group);
    } else if (!layout.isNull()) {
        qDebug() << "Setting keyboard layout to - layout:" << layout
                 << "variant:" << variant;
        widget.setKeyboardLayout(layout, variant);
    }

    mainWindow.setCentralWidget(&widget);
    mainWindow.show();
    qDebug() << "Main window displayed successfully";

    int ret = app.exec();
    qDebug() << "Application exiting with code:" << ret;
    return ret;
}
