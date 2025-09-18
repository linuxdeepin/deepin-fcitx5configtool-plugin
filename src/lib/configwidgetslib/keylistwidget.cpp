/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "keylistwidget.h"
#include "logging.h"

#include <QHBoxLayout>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <fcitx-utils/i18n.h>
#include <fcitxqtkeysequencewidget.h>

namespace fcitx {
namespace kcm {

KeyListWidget::KeyListWidget(QWidget *parent) : QWidget(parent) {
    qCDebug(KCM_FCITX5) << "KeyListWidget created";
    auto layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    keysLayout_ = new QVBoxLayout;
    keysLayout_->setContentsMargins(0,0,0,0);
    auto subLayout = new QVBoxLayout;

    addButton_ = new QToolButton;
    addButton_->setAutoRaise(true);
    addButton_->setIcon(QIcon::fromTheme(
        "list-add-symbolic",
        style()->standardIcon(QStyle::SP_FileDialogNewFolder)));
    addButton_->setText(_("Add"));
    addButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(addButton_, &QToolButton::clicked, this, [this]() {
        addKey(Key());
        Q_EMIT keyChanged();
    });

    layout->addLayout(keysLayout_);
    subLayout->addWidget(addButton_, 0, Qt::AlignTop);
    // subLayout->addStretch(1);
    layout->addLayout(subLayout);

    setLayout(layout);

    // Add an empty one.
    addKey();
    qCDebug(KCM_FCITX5) << "Exiting KeyListWidget constructor";
}

void KeyListWidget::addKey(fcitx::Key key) {
    qCDebug(KCM_FCITX5) << "Adding key";
    auto keyWidget = new FcitxQtKeySequenceWidget;
    keyWidget->setClearButtonShown(false);
    keyWidget->setKeySequence({key});
    keyWidget->setModifierlessAllowed(modifierLess_);
    keyWidget->setModifierOnlyAllowed(modifierOnly_);
    auto widget = new QWidget;
    auto layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(keyWidget);
    auto removeButton = new QToolButton;
    removeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    removeButton->setIcon(QIcon::fromTheme(
        "list-remove-symbolic", style()->standardIcon(QStyle::SP_TrashIcon)));
    removeButton->setText(_("Remove"));
    removeButton->setVisible(showRemoveButton());
    layout->addWidget(removeButton);
    widget->setLayout(layout);
    connect(removeButton, &QPushButton::clicked, widget, [widget, this]() {
        auto idx = keysLayout_->indexOf(widget);
        if (removeKeyAt(idx)) {
            Q_EMIT keyChanged();
        }
    });
    connect(keyWidget, &FcitxQtKeySequenceWidget::keySequenceChanged, this,
            &KeyListWidget::keyChanged);
    connect(this, &KeyListWidget::keyChanged, removeButton,
            [this, removeButton]() {
                removeButton->setVisible(showRemoveButton());
            });
    keysLayout_->addWidget(widget);

    qCDebug(KCM_FCITX5) << "Exiting addKey";
}

void KeyListWidget::setKeys(const QList<fcitx::Key> &keys) {
    qCDebug(KCM_FCITX5) << "Setting keys, count:" << keys.size()
                       << "current keys count:" << keysLayout_->count();
    while (keysLayout_->count() > 1) {
        removeKeyAt(0);
    }
    removeKeyAt(0);

    bool first = true;
    for (auto key : keys) {
        if (first) {
            first = false;
            keysLayout_->itemAt(0)
                ->widget()
                ->findChild<FcitxQtKeySequenceWidget *>()
                ->setKeySequence({key});
        } else {
            addKey(key);
        }
    }
    Q_EMIT keyChanged();
    qCDebug(KCM_FCITX5) << "Exiting setKeys";
}

QList<fcitx::Key> KeyListWidget::keys() const {
    qCDebug(KCM_FCITX5) << "Entering keys";
    QList<fcitx::Key> result;
    for (int i = 0; i < keysLayout_->count(); i++) {
        if (auto keyWidget = keysLayout_->itemAt(i)
                                 ->widget()
                                 ->findChild<FcitxQtKeySequenceWidget *>()) {
            if (keyWidget->keySequence().isEmpty()) {
                continue;
            }
            auto &key = keyWidget->keySequence()[0];
            if (key.isValid() && !result.contains(key)) {
                result << keyWidget->keySequence()[0];
            }
        }
    }
    qCDebug(KCM_FCITX5) << "Exiting keys with" << result.size() << "keys.";
    return result;
}

void KeyListWidget::setAllowModifierLess(bool value) {
    qCDebug(KCM_FCITX5) << "Setting allow modifier less from:" << modifierLess_
                       << "to:" << value;
    if (value == modifierLess_) {
        qCDebug(KCM_FCITX5) << "Modifier less value unchanged";
        return;
    }

    modifierLess_ = value;

    for (int i = 0; i < keysLayout_->count(); i++) {
        if (auto keyWidget = keysLayout_->itemAt(i)
                                 ->widget()
                                 ->findChild<FcitxQtKeySequenceWidget *>()) {
            keyWidget->setModifierlessAllowed(modifierLess_);
        }
    }
    qCDebug(KCM_FCITX5) << "Exiting setAllowModifierLess";
}

void KeyListWidget::setAllowModifierOnly(bool value) {
    qCDebug(KCM_FCITX5) << "Setting allow modifier only from:" << modifierOnly_
                       << "to:" << value;
    if (value == modifierOnly_) {
        qCDebug(KCM_FCITX5) << "Modifier only value unchanged";
        return;
    }

    modifierOnly_ = value;

    for (int i = 0; i < keysLayout_->count(); i++) {
        if (auto keyWidget = keysLayout_->itemAt(i)
                                 ->widget()
                                 ->findChild<FcitxQtKeySequenceWidget *>()) {
            keyWidget->setModifierOnlyAllowed(modifierOnly_);
        }
    }
    qCDebug(KCM_FCITX5) << "Exiting setAllowModifierOnly";
}

bool KeyListWidget::removeKeyAt(int idx) {
    qCDebug(KCM_FCITX5) << "Removing key at index:" << idx
                       << "total keys:" << keysLayout_->count();
    if (idx < 0 || idx > keysLayout_->count()) {
        qCWarning(KCM_FCITX5) << "Invalid index for key removal:" << idx
                             << "valid range: 0-" << keysLayout_->count();
        return false;
    }
    auto widget = keysLayout_->itemAt(idx)->widget();
    if (keysLayout_->count() == 1) {
        qCDebug(KCM_FCITX5) << "Removing last key";
        keysLayout_->itemAt(0)
            ->widget()
            ->findChild<FcitxQtKeySequenceWidget *>()
            ->setKeySequence(QList<Key>());
    } else {
        qCDebug(KCM_FCITX5) << "Removing key at index:" << idx;
        keysLayout_->removeWidget(widget);
        delete widget;
    }
    qCDebug(KCM_FCITX5) << "Exiting removeKeyAt";
    return true;
}

bool KeyListWidget::showRemoveButton() const {
    // qCDebug(KCM_FCITX5) << "Showing remove button, count:" << keysLayout_->count();
    return keysLayout_->count() > 1 ||
           (keysLayout_->count() == 1 &&
            keysLayout_->itemAt(0)
                ->widget()
                ->findChild<FcitxQtKeySequenceWidget *>()
                ->keySequence()
                .size());
}

void KeyListWidget::resizeEvent(QResizeEvent *event) {
    // qCDebug(KCM_FCITX5) << "Entering resizeEvent";
    if (keysLayout_->count() > 0) {
        // qCDebug(KCM_FCITX5) << "Resizing add button to match key widget height";
        addButton_->setMinimumHeight(
            keysLayout_->itemAt(0)
                ->widget()
                ->findChild<FcitxQtKeySequenceWidget *>()
                ->height());
        addButton_->setMaximumHeight(addButton_->minimumHeight());
    }

    QWidget::resizeEvent(event);
    // qCDebug(KCM_FCITX5) << "Exiting resizeEvent";
}

} // namespace kcm
} // namespace fcitx
