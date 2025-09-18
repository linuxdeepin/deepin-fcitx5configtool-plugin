/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "logging.h"
#include <KFontChooser>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>

#include "font.h"
#include "fontbutton.h"

namespace fcitx {
namespace kcm {

FontButton::FontButton(QWidget *parent) : QWidget(parent) {
    qCDebug(KCM_FCITX5) << "FontButton created with parent:" << parent;
    setupUi(this);
    connect(fontSelectButton, &QPushButton::clicked, this,
            &FontButton::selectFont);
}

FontButton::~FontButton() {
    // qCDebug(KCM_FCITX5) << "FontButton destroyed, current font:" << font_.toString();
}

const QFont &FontButton::font()
{
    // qCDebug(KCM_FCITX5) << "Returning font:" << font_.toString();
    return font_;
}

QString FontButton::fontName()
{
    // qCDebug(KCM_FCITX5) << "Returning font name:" << fontPreviewLabel->text();
    return fontPreviewLabel->text();
}

void FontButton::setFont(const QFont &font) {
    // qCDebug(KCM_FCITX5) << "Setting font:" << font.toString()
    //                    << "previous font:" << font_.toString();
    font_ = font;
    if (font.family() != font_.family()) {
        // qCDebug(KCM_FCITX5) << "Font family changed from:" << font.family()
        //                    << "to:" << font_.family();
        Q_EMIT fontChanged(font_);
    }
    fontPreviewLabel->setText(fontToString(font_));
    fontPreviewLabel->setFont(font_);
    // qCDebug(KCM_FCITX5) << "Exiting FontButton::setFont.";
}

void FontButton::selectFont() {
    qCDebug(KCM_FCITX5) << "Opening font selection dialog with current font:"
                       << font_.toString();
    QDialog dialog(NULL);
    KFontChooser *chooser = new KFontChooser(&dialog);
    chooser->setFont(font_);
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialog.setLayout(dialogLayout);
    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText(_("&OK"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(_("&Cancel"));
    dialogLayout->addWidget(chooser);
    dialogLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        qCDebug(KCM_FCITX5) << "Font selected:" << chooser->font().toString()
                           << "previous font:" << font_.toString();
        setFont(chooser->font());
    } else {
        qCDebug(KCM_FCITX5) << "Font selection canceled, keeping current font:"
                           << font_.toString();
    }
}

} // namespace kcm
} // namespace fcitx
