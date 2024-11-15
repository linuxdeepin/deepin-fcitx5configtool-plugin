// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FCITX5CONFIGTOOL_H
#define FCITX5CONFIGTOOL_H

#include "fcitx5configproxy.h"

#include <QObject>
#include <QVariant>
#include <QVariantMap>

namespace deepin {
namespace fcitx5configtool {
class Fcitx5ConfigToolWorkerPrivate;
class Fcitx5ConfigToolWorker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Fcitx5ConfigProxy* fcitx5ConfigProxy READ fcitx5ConfigProxy NOTIFY requestConfigFinished)

public:
    explicit Fcitx5ConfigToolWorker(QObject *parent = nullptr);

    Fcitx5ConfigProxy *fcitx5ConfigProxy() const;

Q_SIGNALS:
    void requestConfigFinished();

protected Q_SLOTS:
    void init();

private:
    friend class Fcitx5ConfigToolWorkerPrivate;
    Fcitx5ConfigToolWorkerPrivate *const d;
};
}   // namespace fcitx5configtool
}   // namespace deepin

#endif   // FCITX5CONFIGTOOL_H
