/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "logging.h"
#include "erroroverlay.h"
#include "dbusprovider.h"
#include "ui_erroroverlay.h"
#include <QAbstractButton>
#include <QIcon>
#include <fcitx-utils/standardpath.h>

namespace fcitx {
namespace kcm {

ErrorOverlay::ErrorOverlay(DBusProvider *dbus, QWidget *parent)
    : QWidget(parent), ui_(std::make_unique<Ui::ErrorOverlay>()),
      baseWidget_(parent) {
    qCDebug(KCM_FCITX5) << "ErrorOverlay created for widget:" << parent;
    ui_->setupUi(this);
    setVisible(false);

    baseWidget_->installEventFilter(this);
    ui_->pixmapLabel->setPixmap(
        QIcon::fromTheme("dialog-error-symbolic").pixmap(64));

    connect(baseWidget_, &QObject::destroyed, this, &QObject::deleteLater);
    connect(dbus, &DBusProvider::availabilityChanged, this,
            &ErrorOverlay::availabilityChanged);

    connect(ui_->runFcitxButton, &QAbstractButton::pressed, this,
            &ErrorOverlay::runFcitx5);
    availabilityChanged(dbus->available());
    qCDebug(KCM_FCITX5) << "Exiting ErrorOverlay constructor";
}

ErrorOverlay::~ErrorOverlay() {
    qCDebug(KCM_FCITX5) << "ErrorOverlay destroyed for widget:" << baseWidget_;
}

void ErrorOverlay::availabilityChanged(bool avail) {
    qCDebug(KCM_FCITX5) << "DBus availability changed:" << avail
                       << "current enabled:" << enabled_;
    const bool newEnabled = !avail;
    if (enabled_ != newEnabled) {
        enabled_ = newEnabled;
        setVisible(newEnabled);
        if (newEnabled) {
            reposition();
        }
    }
    qCDebug(KCM_FCITX5) << "Exiting availabilityChanged";
}

void ErrorOverlay::runFcitx5() {
    qCDebug(KCM_FCITX5) << "Attempting to start Fcitx5 from path:"
                       << QString::fromStdString(StandardPath::fcitxPath("bindir", "fcitx5"));
    QProcess::startDetached(
        QString::fromStdString(StandardPath::fcitxPath("bindir", "fcitx5")),
        QStringList());
}

void ErrorOverlay::reposition() {
    qCDebug(KCM_FCITX5) << "Repositioning";
    if (!baseWidget_) {
        qCWarning(KCM_FCITX5) << "Base widget is null, cannot reposition overlay";
        return;
    }

    // follow base widget visibility
    // needed eg. in tab widgets
    if (!baseWidget_->isVisible()) {
        qCDebug(KCM_FCITX5) << "Base widget is not visible, hiding overlay.";
        hide();
        return;
    }

    show();

    // follow position changes
    const QPoint topLevelPos = baseWidget_->mapTo(window(), QPoint(0, 0));
    const QPoint parentPos = parentWidget()->mapFrom(window(), topLevelPos);
    move(parentPos);

    // follow size changes
    // TODO: hide/scale icon if we don't have enough space
    resize(baseWidget_->size());
    raise();
    qCDebug(KCM_FCITX5) << "Exiting reposition";
}

bool ErrorOverlay::eventFilter(QObject *object, QEvent *event) {
    // qCDebug(KCM_FCITX5) << "Event filtered";
    if (enabled_ && object == baseWidget_ &&
        (event->type() == QEvent::Move || event->type() == QEvent::Resize ||
         event->type() == QEvent::Show || event->type() == QEvent::Hide ||
         event->type() == QEvent::ParentChange)) {
        // qCDebug(KCM_FCITX5) << "Filtered event" << event->type() << "for base widget, repositioning overlay.";
        reposition();
    }
    return QWidget::eventFilter(object, event);
}

} // namespace kcm
} // namespace fcitx
