/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "optionwidget.h"
#include "config.h"
#include "configwidget.h"
#include "font.h"
#include "fontbutton.h"
#include "keylistwidget.h"
#include "listoptionwidget.h"
#include "logging.h"
#include "varianthelper.h"
#include <KColorButton>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QLineEdit>
#include <QPointer>
#include <QProcess>
#include <QPushButton>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <fcitx-utils/color.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>
#include <fcitxqtkeysequencewidget.h>

namespace fcitx {
namespace kcm {

namespace {

class IntegerOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    IntegerOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                        QWidget *parent)
        : OptionWidget(path, parent), spinBox_(new QSpinBox),
          defaultValue_(option.defaultValue().variant().toString().toInt()) {
        qCDebug(KCM_FCITX5) << "Create IntegerOptionWidget";
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(0,0,0,0);

        spinBox_ = new QSpinBox;
        spinBox_->setMaximum(INT_MAX);
        spinBox_->setMinimum(INT_MIN);
        if (option.properties().contains("IntMax")) {
            // qCDebug(KCM_FCITX5) << "option contains IntMax";
            auto max = option.properties().value("IntMax");
            if (max.type() == QVariant::String) {
                // qCDebug(KCM_FCITX5) << "type is String";
                spinBox_->setMaximum(max.toInt());
            }
        }
        if (option.properties().contains("IntMin")) {
            // qCDebug(KCM_FCITX5) << "option contains IntMin";
            auto min = option.properties().value("IntMin");
            if (min.type() == QVariant::String) {
                // qCDebug(KCM_FCITX5) << "type is String";
                spinBox_->setMinimum(min.toInt());
            }
        }
        connect(spinBox_, qOverload<int>(&QSpinBox::valueChanged), this,
                &OptionWidget::valueChanged);
        layout->addWidget(spinBox_);
        setLayout(layout);
        qCDebug(KCM_FCITX5) << "IntegerOptionWidget for path" << path << "created.";
    }

    void readValueFrom(const QVariantMap &map) override {
        // qCDebug(KCM_FCITX5) << "Entering IntegerOptionWidget::readValueFrom for path:" << path();
        auto value = readString(map, path());
        if (value.isNull()) {
            // qCDebug(KCM_FCITX5) << "Read default value for IntegerOptionWidget:" << defaultValue_;
            spinBox_->setValue(defaultValue_);
        } else {
            // qCDebug(KCM_FCITX5) << "Read value for IntegerOptionWidget:" << value;
            spinBox_->setValue(value.toInt());
        }
        // qCDebug(KCM_FCITX5) << "Exiting IntegerOptionWidget::readValueFrom for path:" << path();
    }

    void writeValueTo(QVariantMap &map) override {
        // qCDebug(KCM_FCITX5) << "Entering IntegerOptionWidget::writeValueTo for path:" << path();
        int value = spinBox_->value();

        writeVariant(map, path(), QString::number(value));
        // qCDebug(KCM_FCITX5) << "Exiting IntegerOptionWidget::writeValueTo for path:" << path();
    }

    void restoreToDefault() override {
        // qCDebug(KCM_FCITX5) << "Restore IntegerOptionWidget to default:" << defaultValue_;
        spinBox_->setValue(defaultValue_);
    }

private:
    QSpinBox *spinBox_;
    int defaultValue_;
};

class StringOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    StringOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                       QWidget *parent)
        : OptionWidget(path, parent), lineEdit_(new QLineEdit),
          defaultValue_(option.defaultValue().variant().toString()) {
        qCDebug(KCM_FCITX5) << "Create StringOptionWidget";
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(0,0,0,0);

        lineEdit_ = new QLineEdit;
        connect(lineEdit_, &QLineEdit::textChanged, this,
                &OptionWidget::valueChanged);
        layout->addWidget(lineEdit_);
        setLayout(layout);
        qCDebug(KCM_FCITX5) << "StringOptionWidget for path" << path << "created.";
    }

    void readValueFrom(const QVariantMap &map) override {
        // qCDebug(KCM_FCITX5) << "Entering StringOptionWidget::readValueFrom for path:" << path();
        auto value = readString(map, path());

        lineEdit_->setText(value);
        // qCDebug(KCM_FCITX5) << "Exiting StringOptionWidget::readValueFrom for path:" << path();
    }

    void writeValueTo(QVariantMap &map) override {
        // qCDebug(KCM_FCITX5) << "Entering StringOptionWidget::writeValueTo for path:" << path();
        QString value = lineEdit_->text();

        writeVariant(map, path(), value);
        // qCDebug(KCM_FCITX5) << "Exiting StringOptionWidget::writeValueTo for path:" << path();
    }

    void restoreToDefault() override {
        // qCDebug(KCM_FCITX5) << "Restore StringOptionWidget to default:" << defaultValue_;
        lineEdit_->setText(defaultValue_);
    }

private:
    QLineEdit *lineEdit_;
    QString defaultValue_;
};

class FontOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    FontOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                     QWidget *parent)
        : OptionWidget(path, parent), fontButton_(new FontButton),
          defaultValue_(option.defaultValue().variant().toString()) {
        qCDebug(KCM_FCITX5) << "Create FontOptionWidget";
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(0,0,0,0);

        connect(fontButton_, &FontButton::fontChanged, this,
                &OptionWidget::valueChanged);
        layout->addWidget(fontButton_);
        setLayout(layout);
        qCDebug(KCM_FCITX5) << "FontOptionWidget for path" << path << "created.";
    }

    void readValueFrom(const QVariantMap &map) override {
        // qCDebug(KCM_FCITX5) << "Entering FontOptionWidget::readValueFrom for path:" << path();
        auto value = readString(map, path());

        fontButton_->setFont(parseFont(value));
        // qCDebug(KCM_FCITX5) << "Exiting FontOptionWidget::readValueFrom for path:" << path();
    }

    void writeValueTo(QVariantMap &map) override {
        // qCDebug(KCM_FCITX5) << "Entering FontOptionWidget::writeValueTo for path:" << path();
        QString fontName = fontButton_->fontName();

        writeVariant(map, path(), fontName);
        // qCDebug(KCM_FCITX5) << "Exiting FontOptionWidget::writeValueTo for path:" << path();
    }

    void restoreToDefault() override {
        // qCDebug(KCM_FCITX5) << "Restore FontOptionWidget to default for path" << path() << "with value" << defaultValue_;
        fontButton_->setFont(parseFont(defaultValue_));
    }

private:
    FontButton *fontButton_;
    QString defaultValue_;
};

class BooleanOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    BooleanOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                        QWidget *parent)
        : OptionWidget(path, parent), checkBox_(new QCheckBox),
          defaultValue_(option.defaultValue().variant().toString() == "True") {
        qCDebug(KCM_FCITX5) << "Create BooleanOptionWidget";
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(0,0,0,0);

        connect(checkBox_, &QCheckBox::clicked, this,
                &OptionWidget::valueChanged);
        checkBox_->setText(option.description());
        layout->addWidget(checkBox_);
        setLayout(layout);
        qCDebug(KCM_FCITX5) << "BooleanOptionWidget for path" << path << "created.";
    }

    void readValueFrom(const QVariantMap &map) override {
        // qCDebug(KCM_FCITX5) << "Entering BooleanOptionWidget::readValueFrom for path:" << path();
        bool value = readBool(map, path());

        checkBox_->setChecked(value);
        // qCDebug(KCM_FCITX5) << "Exiting BooleanOptionWidget::readValueFrom for path:" << path();
    }

    void writeValueTo(QVariantMap &map) override {
        // qCDebug(KCM_FCITX5) << "Entering BooleanOptionWidget::writeValueTo for path:" << path();
        bool isChecked = checkBox_->isChecked();
        QString value = isChecked ? "True" : "False";

        writeVariant(map, path(), value);
        // qCDebug(KCM_FCITX5) << "Exiting BooleanOptionWidget::writeValueTo for path:" << path();
    }

    void restoreToDefault() override {
        // qCDebug(KCM_FCITX5) << "Restore BooleanOptionWidget to default for path" << path() << "with value" << defaultValue_;
        checkBox_->setChecked(defaultValue_);
    }

private:
    QCheckBox *checkBox_;
    bool defaultValue_;
};

class KeyListOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    KeyListOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                        QWidget *parent)
        : OptionWidget(path, parent), keyListWidget_(new KeyListWidget) {
        qCDebug(KCM_FCITX5) << "Create KeyListOptionWidget";
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(0,0,0,0);

        keyListWidget_ = new KeyListWidget(this);

        keyListWidget_->setAllowModifierLess(
            readString(option.properties(),
                       "ListConstrain/AllowModifierLess") == "True");
        keyListWidget_->setAllowModifierOnly(
            readString(option.properties(),
                       "ListConstrain/AllowModifierOnly") == "True");
        connect(keyListWidget_, &KeyListWidget::keyChanged, this,
                &OptionWidget::valueChanged);
        layout->addWidget(keyListWidget_);

        auto variant = option.defaultValue().variant();
        QVariantMap map;
        if (variant.canConvert<QDBusArgument>()) {
            // qCDebug(KCM_FCITX5) << "variant can convert to QDBusArgument";
            auto argument = qvariant_cast<QDBusArgument>(variant);
            argument >> map;
        }
        defaultValue_ = readValue(map, "");
        setLayout(layout);
        qCDebug(KCM_FCITX5) << "KeyListOptionWidget for path" << path << "created.";
    }

    void readValueFrom(const QVariantMap &map) override {
        auto keys = readValue(map, path());
        // qCDebug(KCM_FCITX5) << "Read keys for KeyListOptionWidget, count:" << keys.size();
        keyListWidget_->setKeys(keys);
    }

    void writeValueTo(QVariantMap &map) override {
        auto keys = keyListWidget_->keys();
        // qCDebug(KCM_FCITX5) << "Write keys for KeyListOptionWidget, count:" << keys.size();
        int i = 0;
        for (auto &key : keys) {
            auto value = QString::fromUtf8(key.toString().data());
            writeVariant(map, QString("%1/%2").arg(path()).arg(i), value);
            i++;
        }
        if (keys.empty()) {
            // qCDebug(KCM_FCITX5) << "Write empty keys for KeyListOptionWidget";
            writeVariant(map, path(), QVariantMap());
        }
    }

    void restoreToDefault() override {
        // qCDebug(KCM_FCITX5) << "Restore KeyListOptionWidget to default, count:" << defaultValue_.size();
        keyListWidget_->setKeys(defaultValue_);
    }

private:
    QList<fcitx::Key> readValue(const QVariantMap &map, const QString &path) {
        // qCDebug(KCM_FCITX5) << "Read keys for KeyListOptionWidget, path:" << path;
        int i = 0;
        QList<Key> keys;
        while (true) {
            auto value = readString(map, QString("%1%2%3")
                                             .arg(path)
                                             .arg(path.isEmpty() ? "" : "/")
                                             .arg(i));
            if (value.isNull()) {
                // qCDebug(KCM_FCITX5) << "Read empty key for KeyListOptionWidget";
                break;
            }
            keys << Key(value.toUtf8().constData());
            i++;
        }
        // qCDebug(KCM_FCITX5) << "Read keys for KeyListOptionWidget, count:" << keys.size();
        return keys;
    }

    KeyListWidget *keyListWidget_;
    QList<fcitx::Key> defaultValue_;
};

class KeyOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    KeyOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                    QWidget *parent)
        : OptionWidget(path, parent),
          keyWidget_(new FcitxQtKeySequenceWidget(this)),
          defaultValue_(
              option.defaultValue().variant().toString().toUtf8().constData()) {
        qCDebug(KCM_FCITX5) << "Create KeyOptionWidget";
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(0,0,0,0);

        keyWidget_->setModifierlessAllowed(
            readBool(option.properties(), "AllowModifierLess"));
        keyWidget_->setModifierOnlyAllowed(
            readBool(option.properties(), "AllowModifierOnly"));

        connect(keyWidget_, &FcitxQtKeySequenceWidget::keySequenceChanged, this,
                &OptionWidget::valueChanged);
        layout->addWidget(keyWidget_);
        setLayout(layout);
        qCDebug(KCM_FCITX5) << "KeyOptionWidget for path" << path << "created.";
    }

    void readValueFrom(const QVariantMap &map) override {
        // qCDebug(KCM_FCITX5) << "Entering KeyOptionWidget::readValueFrom for path:" << path();
        Key key;
        auto value = readString(map, path());
        key = Key(value.toUtf8().constData());
        // qCDebug(KCM_FCITX5) << "Read key for KeyOptionWidget:" << value;
        keyWidget_->setKeySequence({key});
        // qCDebug(KCM_FCITX5) << "Exiting KeyOptionWidget::readValueFrom for path:" << path();
    }

    void writeValueTo(QVariantMap &map) override {
        // qCDebug(KCM_FCITX5) << "Entering KeyOptionWidget::writeValueTo for path:" << path();
        auto keys = keyWidget_->keySequence();
        Key key;
        if (keys.size()) {
            key = keys[0];
        }
        auto value = QString::fromUtf8(key.toString().data());
        // qCDebug(KCM_FCITX5) << "Write key for KeyOptionWidget:" << value;
        writeVariant(map, path(), value);
        // qCDebug(KCM_FCITX5) << "Exiting KeyOptionWidget::writeValueTo for path:" << path();
    }

    void restoreToDefault() override {
        // qCDebug(KCM_FCITX5) << "Restore KeyOptionWidget";
        keyWidget_->setKeySequence({defaultValue_});
    }

private:
    FcitxQtKeySequenceWidget *keyWidget_;
    fcitx::Key defaultValue_;
};

class EnumOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    EnumOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                     QWidget *parent)
        : OptionWidget(path, parent), comboBox_(new QComboBox),
          toolButton_(new QToolButton) {
        qCDebug(KCM_FCITX5) << "Create EnumOptionWidget";
        auto *layout = new QHBoxLayout;
        toolButton_->setIcon(QIcon::fromTheme("preferences-system-symbolic"));
        layout->setContentsMargins(0,0,0,0);

        int i = 0;
        while (true) {
            auto value =
                readString(option.properties(), QString("Enum/%1").arg(i));
            if (value.isNull()) {
                break;
            }
            auto text =
                readString(option.properties(), QString("EnumI18n/%1").arg(i));
            if (text.isEmpty()) {
                text = value;
            }
            auto subConfigPath = readString(option.properties(),
                                            QString("SubConfigPath/%1").arg(i));
            comboBox_->addItem(text, value);
            comboBox_->setItemData(i, subConfigPath, subConfigPathRole);
            i++;
        }
        layout->addWidget(comboBox_);
        layout->addWidget(toolButton_);
        setLayout(layout);

        connect(comboBox_, qOverload<int>(&QComboBox::currentIndexChanged),
                this, &OptionWidget::valueChanged);

        connect(comboBox_, qOverload<int>(&QComboBox::currentIndexChanged),
                this, [this]() {
                    toolButton_->setVisible(
                        !comboBox_->currentData(subConfigPathRole)
                             .toString()
                             .isEmpty());
                });

        connect(toolButton_, &QToolButton::clicked, this, [this]() {
            qCDebug(KCM_FCITX5) << "EnumOptionWidget tool button clicked for path:" << this->path();
            ConfigWidget *configWidget = getConfigWidget(this);
            if (!configWidget) {
                qCWarning(KCM_FCITX5) << "ConfigWidget not found for EnumOptionWidget at path:" << this->path();
                return;
            }
            QPointer<QDialog> dialog = ConfigWidget::configDialog(
                this, configWidget->dbus(),
                comboBox_->currentData(subConfigPathRole).toString(),
                comboBox_->currentText());
            dialog->exec();
            delete dialog;
        });

        defaultValue_ = option.defaultValue().variant().toString();
        qCDebug(KCM_FCITX5) << "EnumOptionWidget for path" << path << "created.";
    }

    void readValueFrom(const QVariantMap &map) override {
        // qCDebug(KCM_FCITX5) << "Entering EnumOptionWidget::readValueFrom for path:" << path();
        auto value = readString(map, path());
        auto idx = comboBox_->findData(value);
        if (idx < 0) {
            // qCDebug(KCM_FCITX5) << "Read value for EnumOptionWidget not found, using default value:" << defaultValue_;
            idx = comboBox_->findData(defaultValue_);
        }
        // qCDebug(KCM_FCITX5) << "Read value for EnumOptionWidget:" << value
        //                    << ", selected index:" << idx;
        comboBox_->setCurrentIndex(idx);
        toolButton_->setVisible(
            !comboBox_->currentData(subConfigPathRole).toString().isEmpty());
        // qCDebug(KCM_FCITX5) << "Exiting EnumOptionWidget::readValueFrom for path:" << path();
    }

    void writeValueTo(QVariantMap &map) override {
        // qCDebug(KCM_FCITX5) << "Entering EnumOptionWidget::writeValueTo for path:" << path();
        QString value = comboBox_->currentData().toString();
        // qCDebug(KCM_FCITX5) << "Write value for EnumOptionWidget:" << value;
        writeVariant(map, path(), value);
        // qCDebug(KCM_FCITX5) << "Exiting EnumOptionWidget::writeValueTo for path:" << path();
    }

    void restoreToDefault() override {
        // qCDebug(KCM_FCITX5) << "Entering EnumOptionWidget::restoreToDefault for path:" << path();
        auto idx = comboBox_->findData(defaultValue_);
        // qCDebug(KCM_FCITX5) << "Restore EnumOptionWidget to default, index:" << idx;
        comboBox_->setCurrentIndex(idx);
        // qCDebug(KCM_FCITX5) << "Exiting EnumOptionWidget::restoreToDefault for path:" << path();
    }

private:
    QComboBox *comboBox_;
    QToolButton *toolButton_;
    QString defaultValue_;
    inline static constexpr int subConfigPathRole = Qt::UserRole + 1;
};

class ColorOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    ColorOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                       QWidget *parent)
        : OptionWidget(path, parent), colorButton_(new KColorButton) {
        qCDebug(KCM_FCITX5) << "Create ColorOptionWidget";
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(0,0,0,0);
        layout->addWidget(colorButton_);
        colorButton_->setAlphaChannelEnabled(true);
        setLayout(layout);
        connect(colorButton_, &KColorButton::changed, this,
                &OptionWidget::valueChanged);

        try {
            defaultValue_.setFromString(
                option.defaultValue().variant().toString().toStdString());
        } catch (...) {
        }
        qCDebug(KCM_FCITX5) << "ColorOptionWidget for path" << path << "created.";
    }

    void readValueFrom(const QVariantMap &map) override {
        // qCDebug(KCM_FCITX5) << "Entering ColorOptionWidget::readValueFrom for path:" << path();
        auto value = readString(map, path());
        Color color;
        try {
            color.setFromString(value.toStdString());
            // qCDebug(KCM_FCITX5) << "Read color for ColorOptionWidget:" << value;
        } catch (...) {
            color = defaultValue_;
            // qCDebug(KCM_FCITX5) << "Using default color for ColorOptionWidget";
        }
        QColor qcolor;
        qcolor.setRedF(color.redF());
        qcolor.setGreenF(color.greenF());
        qcolor.setBlueF(color.blueF());
        qcolor.setAlphaF(color.alphaF());
        colorButton_->setColor(qcolor);
        // qCDebug(KCM_FCITX5) << "Exiting ColorOptionWidget::readValueFrom for path:" << path();
    }

    void writeValueTo(QVariantMap &map) override {
        // qCDebug(KCM_FCITX5) << "Entering ColorOptionWidget::writeValueTo for path:" << path();
        auto color = colorButton_->color();
        Color fcitxColor;
        fcitxColor.setRedF(color.redF());
        fcitxColor.setGreenF(color.greenF());
        fcitxColor.setBlueF(color.blueF());
        fcitxColor.setAlphaF(color.alphaF());
        writeVariant(map, path(),
                     QString::fromStdString(fcitxColor.toString()));
        // qCDebug(KCM_FCITX5) << "Exiting ColorOptionWidget::writeValueTo for path:" << path();
    }

    void restoreToDefault() override {
        // qCDebug(KCM_FCITX5) << "Restore ColorOptionWidget to default for path:" << path();
        QColor qcolor;
        qcolor.setRedF(defaultValue_.redF());
        qcolor.setGreenF(defaultValue_.greenF());
        qcolor.setBlueF(defaultValue_.blueF());
        qcolor.setAlphaF(defaultValue_.alphaF());
        colorButton_->setColor(qcolor);
    }

private:
    KColorButton *colorButton_;
    Color defaultValue_;
};

class ExternalOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    ExternalOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                          QWidget *parent)
        : OptionWidget(path, parent),
          uri_(readString(option.properties(), "External")),
          launchSubConfig_(readBool(option.properties(), "LaunchSubConfig")) {
        qCDebug(KCM_FCITX5) << "Create ExternalOptionWidget for path:" << path
                           << "URI:" << uri_
                           << "launchSubConfig:" << launchSubConfig_;
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(0,0,0,0);

        button_ = new QToolButton(this);
        button_->setIcon(QIcon::fromTheme("preferences-system-symbolic"));
        button_->setText(_("Configure"));
        layout->addWidget(button_);
        setLayout(layout);

        connect(
            button_, &QPushButton::clicked, this,
            [this, parent, name = option.name()]() {
                if (launchSubConfig_) {
                    ConfigWidget *configWidget = getConfigWidget(this);
                    if (!configWidget) {
                        return;
                    }
                    QPointer<QDialog> dialog = ConfigWidget::configDialog(
                        this, configWidget->dbus(), uri_, name);
                    dialog->exec();
                    delete dialog;
                } else if (uri_.startsWith("fcitx://config/addon/")) {
                    QString wrapperPath = FCITX5_QT5_GUI_WRAPPER;
                    if (!QFileInfo(wrapperPath).isExecutable()) {
                        wrapperPath =
                            QString::fromStdString(stringutils::joinPath(
                                StandardPath::global().fcitxPath("libexecdir"),
                                "fcitx5-qt5-gui-wrapper"));
                    }
                    QStringList args;
                    if (QGuiApplication::platformName() == "xcb") {
                        auto wid = parent->winId();
                        if (wid) {
                            args << "-w";
                            args << QString::number(wid);
                        }
                    }
                    args << uri_;
                    qCDebug(KCM_FCITX5) << "Launch: " << wrapperPath << args;
                    QProcess::startDetached(wrapperPath, args);
                } else {
                // Assume this is a program path.
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
                    QStringList args = QProcess::splitCommand(uri_);
                    QString program = args.takeFirst();
                    QProcess::startDetached(program, args);
#else
                    QProcess::startDetached(uri_);
#endif
                }
            });
    }

    void readValueFrom(const QVariantMap &) override {}
    void writeValueTo(QVariantMap &) override {}
    void restoreToDefault() override {}

private:
    QToolButton *button_;
    const QString uri_;
    const bool launchSubConfig_;
};
} // namespace

OptionWidget *OptionWidget::addWidget(QFormLayout *layout,
                                      const fcitx::FcitxQtConfigOption &option,
                                      const QString &path, QWidget *parent) {
    qCDebug(KCM_FCITX5) << "Adding widget for option" << option.name() << "with type" << option.type() << "and path" << path;
    OptionWidget *widget = nullptr;
    if (option.type() == "Integer") {
        // qCDebug(KCM_FCITX5) << "option type is Integer";
        widget = new IntegerOptionWidget(option, path, parent);
        layout->addRow(QString(_("%1:")).arg(option.description()), widget);
    } else if (option.type() == "String") {
        // qCDebug(KCM_FCITX5) << "option type is String";
        const auto isFont = readBool(option.properties(), "Font");
        const auto isEnum = readBool(option.properties(), "IsEnum");
        if (isFont) {
            // qCDebug(KCM_FCITX5) << "option type is Font";
            widget = new FontOptionWidget(option, path, parent);
        } else if (isEnum) {
            widget = new EnumOptionWidget(option, path, parent);
        } else {
            widget = new StringOptionWidget(option, path, parent);
        }
        layout->addRow(QString(_("%1:")).arg(option.description()), widget);
    } else if (option.type() == "Boolean") {
        // qCDebug(KCM_FCITX5) << "option type is Boolean";
        widget = new BooleanOptionWidget(option, path, parent);
        layout->addRow("", widget);
    } else if (option.type() == "Key") {
        // qCDebug(KCM_FCITX5) << "option type is Key";
        widget = new KeyOptionWidget(option, path, parent);
        layout->addRow(QString(_("%1:")).arg(option.description()), widget);
    } else if (option.type() == "List|Key") {
        // qCDebug(KCM_FCITX5) << "option type is List|Key";
        widget = new KeyListOptionWidget(option, path, parent);
        layout->addRow(QString(_("%1:")).arg(option.description()), widget);
    } else if (option.type() == "Enum") {
        // qCDebug(KCM_FCITX5) << "option type is Enum";
        widget = new EnumOptionWidget(option, path, parent);
        layout->addRow(QString(_("%1:")).arg(option.description()), widget);
    } else if (option.type() == "Color") {
        // qCDebug(KCM_FCITX5) << "option type is Color";
        widget = new ColorOptionWidget(option, path, parent);
        layout->addRow(QString(_("%1:")).arg(option.description()), widget);
    } else if (option.type().startsWith("List|")) {
        // qCDebug(KCM_FCITX5) << "option type is List|";
        widget = new ListOptionWidget(option, path, parent);
        layout->addRow(QString(_("%1:")).arg(option.description()), widget);
    } else if (option.type() == "External") {
        // qCDebug(KCM_FCITX5) << "option type is External";
        widget = new ExternalOptionWidget(option, path, parent);
        layout->addRow(QString(_("%1:")).arg(option.description()), widget);
    }
    if (widget) {
        // qCDebug(KCM_FCITX5) << "Adding widget for option" << option.name() << "with path" << path;
        if (option.properties().contains("Tooltip")) {
            // qCDebug(KCM_FCITX5) << "option contains Tooltip";
            widget->setToolTip(option.properties().value("Tooltip").toString());
        }
    }
    return widget;
}

bool OptionWidget::execOptionDialog(QWidget *parent,
                                    const fcitx::FcitxQtConfigOption &option,
                                    QVariant &result) {
    qCDebug(KCM_FCITX5) << "Executing option dialog for" << option.name();
    QPointer<QDialog> dialog = new QDialog(parent);
    dialog->setWindowIcon(QIcon::fromTheme("fcitx"));
    dialog->setWindowTitle(option.description());
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialog->setLayout(dialogLayout);

    ConfigWidget *parentConfigWidget = getConfigWidget(parent);
    OptionWidget *optionWidget = nullptr;
    ConfigWidget *configWidget = nullptr;
    if (parentConfigWidget->description().contains(option.type())) {
        // qCDebug(KCM_FCITX5) << "parentConfigWidget contains option type";
        configWidget =
            new ConfigWidget(parentConfigWidget->description(), option.type(),
                             parentConfigWidget->dbus());
        configWidget->setValue(result);
        dialogLayout->addWidget(configWidget);
    } else {
        // qCDebug(KCM_FCITX5) << "parentConfigWidget does not contain option type";
        QFormLayout *subLayout = new QFormLayout;
        dialogLayout->addLayout(subLayout);
        optionWidget =
            addWidget(subLayout, option, QString("Value"), dialog.data());
        if (!optionWidget) {
            // qCDebug(KCM_FCITX5) << "OptionWidget not found for option" << option.name();
            return false;
        }
        QVariantMap origin;
        origin["Value"] = result;
        optionWidget->readValueFrom(origin);
    }

    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText(_("&OK"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(_("&Cancel"));
    dialogLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    auto ret = dialog->exec();
    bool dialogResult = false;
    if (ret && dialog) {
        qCDebug(KCM_FCITX5) << "Option dialog for" << option.name() << "finished with result" << ret;
        if (optionWidget) {
            // qCDebug(KCM_FCITX5) << "OptionWidget is valid";
            if (optionWidget->isValid()) {
                QVariantMap map;
                optionWidget->writeValueTo(map);
                result = map.value("Value");
                dialogResult = true;
            }
        } else {
            // qCDebug(KCM_FCITX5) << "configWidget is valid";
            result = configWidget->value();
            dialogResult = true;
        }
    }
    qCDebug(KCM_FCITX5) << "Option dialog for" << option.name() << "finished with result" << dialogResult;

    delete dialog;
    return dialogResult;
}

QString OptionWidget::prettify(const fcitx::FcitxQtConfigOption &option,
                               const QVariant &value) {
    qCDebug(KCM_FCITX5) << "Prettifying option" << option.name() << "with value" << value;
    if (option.type() == "Integer") {
        // qCDebug(KCM_FCITX5) << "option type is Integer";
        return value.toString();
    } else if (option.type() == "String") {
        // qCDebug(KCM_FCITX5) << "option type is String";
        return value.toString();
    } else if (option.type() == "Boolean") {
        // qCDebug(KCM_FCITX5) << "option type is Boolean";
        return value.toString() == "True" ? _("Yes") : _("No");
    } else if (option.type() == "Key") {
        // qCDebug(KCM_FCITX5) << "option type is Key";
        return value.toString();
    } else if (option.type() == "Enum") {
        // qCDebug(KCM_FCITX5) << "option type is Enum";
        QMap<QString, QString> enumMap;
        int i = 0;
        while (true) {
            auto value =
                readString(option.properties(), QString("Enum/%1").arg(i));
            if (value.isNull()) {
                break;
            }
            auto text =
                readString(option.properties(), QString("EnumI18n/%1").arg(i));
            if (text.isEmpty()) {
                text = value;
            }
            enumMap[value] = text;
            i++;
        }
        return enumMap.value(value.toString());
    } else if (option.type().startsWith("List|")) {
        // qCDebug(KCM_FCITX5) << "option type is List|";
        int i = 0;
        QStringList strs;
        strs.clear();
        auto subOption = option;
        subOption.setType(option.type().mid(5)); // Remove List|
        while (true) {
            auto subValue = readVariant(value, QString::number(i));
            strs << prettify(subOption, subValue);
            i++;
        }
        return QString(_("[%1]")).arg(strs.join(" "));
    } else {
        // qCDebug(KCM_FCITX5) << "option type is other";
        auto *configWidget = getConfigWidget(this);
        if (configWidget &&
            configWidget->description().contains(option.type())) {
            if (auto key =
                    option.properties().value("ListDisplayOption").toString();
                !key.isEmpty()) {
                const auto &options =
                    *configWidget->description().find(option.type());
                for (const auto &option : options) {
                    if (option.name() == key) {
                        return prettify(option, readVariant(value, key));
                    }
                }
            }
        }
    }
    qCWarning(KCM_FCITX5) << "Could not prettify option" << option.name();
    return QString();
}

} // namespace kcm
} // namespace fcitx

#include "optionwidget.moc"
