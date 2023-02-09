// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _KCM_FCITX_ISO639_H_
#define _KCM_FCITX_ISO639_H_

#include <QMap>
#include <QString>
#include <fcitx-utils/i18n.h>

namespace fcitx {
namespace addim {

class Iso639 {
public:
    Iso639();

    QString query(const QString &code) const {
        auto value = m_iso639_2data.value(code);
        if (!value.isEmpty()) {
            return translateDomain("iso_639-2", value.toUtf8().constData());
        }
        value = m_iso639_3data.value(code);
        if (!value.isEmpty()) {
            return translateDomain("iso_639-3", value.toUtf8().constData());
        }
        value = m_iso639_5data.value(code);
        if (!value.isEmpty()) {
            return translateDomain("iso_639-5", value.toUtf8().constData());
        }
        return value;
    }

private:
    QMap<QString, QString> m_iso639_2data;
    QMap<QString, QString> m_iso639_3data;
    QMap<QString, QString> m_iso639_5data;
};

} // namespace addim
} // namespace fcitx

#endif // _KCM_FCITX_ISO639_H_
