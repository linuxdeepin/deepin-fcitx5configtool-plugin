/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef ADDIM_IMPAGE_H
#define ADDIM_IMPAGE_H

#include "imconfig.h"
#include "dbusprovider.h"
#include "layoutselector.h"
#include <QDBusPendingCallWatcher>
#include <QWidget>
#include <fcitxqtdbustypes.h>
#include <memory>
#include <DSearchEdit>

namespace Ui {
class IMPage;
}


namespace fcitx {
namespace addim {

class AvailIMModel;
class IMProxyModel;
class CurrentIMModel;
class ADDIMDBusProvider;

class BaseWidget;
class IMPage : public QWidget {
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

    void availIMSelectionChanged();
    void currentIMCurrentChanged();
    void selectCurrentIM(const QModelIndex &index);
    void clickCurrentIM(const QModelIndex &index);
    void clickAvailIM(const QModelIndex &index);
    void selectDefaultLayout();

    void moveDownIM();
    void moveUpIM();

    void clickedCloseButton();
    void clickedAddButton();

private:
    int addIM(const QModelIndex &index, QString matchStr = "");
    void checkDefaultLayout();

private:
    std::unique_ptr<Ui::IMPage> ui_;
    DBusProvider *m_dbus;
    IMConfig *m_config;
    Dtk::Widget::DSearchEdit* m_SearchEdit;
    fcitx::addim::LayoutSelector* m_laSelector;

    BaseWidget* m_wleftview;
    BaseWidget* m_wrightupview;
    BaseWidget* m_wrightdownview;
};

class BaseWidget : public QWidget
{
	Q_OBJECT
public:
	BaseWidget(const QString& text, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
protected:
	void paintEvent(QPaintEvent* e) override;
};

}
}

#endif
