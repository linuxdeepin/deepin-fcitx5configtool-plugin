// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ADDIM_IMPAGE_H
#define ADDIM_IMPAGE_H

#include "dbusprovider.h"
#include "imconfig.h"
#include "layoutselector.h"

#include <fcitxqtdbustypes.h>
#include <widgets/buttontuple.h>

#include <DAbstractDialog>
#include <DListView>
#include <DSearchEdit>
#include <DWidget>
#include <DCommandLinkButton>

#include <QDBusPendingCallWatcher>
#include <QWidget>

class QLabel;
class QPushButton;

namespace fcitx {
namespace addim {

class IMPage : public DTK_WIDGET_NAMESPACE::DAbstractDialog
{
    Q_OBJECT
public:
    IMPage(DBusProvider *dbus, IMConfig *config, QWidget *parent);
    ~IMPage();
signals:
    void changed();
    void closeAddIMWindow();

public slots:
    void save();
    void load();
    void defaults();

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void availIMCurrentChanged(const QModelIndex &index);
    void childIMSelectionChanged(const QItemSelection &selection);

    void clickedCloseButton();
    void clickedAddButton();

private:
    DBusProvider *m_dbus;
    IMConfig *m_config;

    QHBoxLayout *m_contentLayout;
    QVBoxLayout *m_leftLayout;
    Dtk::Widget::DSearchEdit *m_searchEdit;
    QFrame *line;
    Dtk::Widget::DListView *m_availIMList;
    QVBoxLayout *m_rightLayout;
    Dtk::Widget::DListView *m_childIMList;
    fcitx::addim::LayoutSelector *m_laSelector;
    Dtk::Widget::DCommandLinkButton *m_findMoreLabel;
    DCC_NAMESPACE::ButtonTuple *m_buttonTuple;
};

}
}

#endif
