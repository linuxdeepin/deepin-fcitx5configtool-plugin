// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QLoggingCategory>
#include <QApplication>

Q_LOGGING_CATEGORY(fcitx5Exec, "fcitx5.configtool.exec")
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>

#include <configwidget.h>
#include <dbusprovider.h>

using namespace fcitx::kcm;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("dcc-fcitx5configtool-exec");
    app.setApplicationVersion("1.0");

    // 创建命令行解析器
    QCommandLineParser parser;
    parser.setApplicationDescription("Fcitx5 Configuration Tool");
    parser.addHelpOption();
    parser.addVersionOption();

    const QString &transPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    QStringList qtTrans = { QDir(transPath).filePath("qt_"), QDir(transPath).filePath("qtbase_") };
    for (const QString &trans : qtTrans) {
        QTranslator *translator = new QTranslator;
        const QString filename = trans + QLocale::system().name();
        bool ret = translator->load(filename);
        qCInfo(fcitx5Exec) << "Loading translator file:" << filename;

        if (!ret) {
            delete translator;
            continue;
        }

        app.installTranslator(translator);
    }

    // 添加命令行选项
    QCommandLineOption uriOption(QStringList() << "u"
                                               << "uri",
                                 "Specify uri",
                                 "uri");
    parser.addOption(uriOption);

    QCommandLineOption titleOption(QStringList() << "t"
                                                 << "title",
                                   "Specify dialog title",
                                   "title");
    parser.addOption(titleOption);

    // 解析命令行参数
    parser.process(app);

    // 获取参数值
    QString uri = parser.value(uriOption);
    QString title = parser.value(titleOption);
    qCInfo(fcitx5Exec) << "Parsed command line arguments - uri:" << uri
                      << "title:" << title;
    if (uri.isEmpty()) {
        qCWarning(fcitx5Exec) << "URI parameter is required but not provided";
        parser.showHelp(1);
    }

    qCDebug(fcitx5Exec) << "Initializing DBusProvider";
    DBusProvider *dbusProvider = new DBusProvider(&app);

    qCInfo(fcitx5Exec) << "Creating config dialog for uri:" << uri;
    QPointer<QDialog> dialog = ConfigWidget::configDialog(nullptr,
                                                          dbusProvider,
                                                          uri,
                                                          title);
    if (!dialog) {
        qCCritical(fcitx5Exec) << "Failed to create config dialog for uri:" << uri;
        return 1;
    }

    return dialog->exec();
}
