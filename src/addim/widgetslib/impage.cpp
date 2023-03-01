// SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#include "impage.h"

#include "model.h"
#include "glo.h"
#include "layoutselector.h"

#include <fcitxqtcontrollerproxy.h>
#include <widgets/buttontuple.h>

#include <DDBusSender>
#include <DDialog>
#include <DGuiApplicationHelper>
#include <DTitlebar>
#include <DFrame>

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QTreeView>
#include <DTableView>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
using namespace Dtk::Gui;
using namespace DCC_NAMESPACE;

namespace fcitx {
namespace addim {

IMPage::IMPage(DBusProvider *dbus, IMConfig *config, QWidget *parent)
    : DAbstractDialog(parent)
    , m_dbus(dbus)
    , m_config(config)
{
    setFixedSize(690, 620);

    auto *wrapperLayout = new QVBoxLayout(this);
    wrapperLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(wrapperLayout);

    DTitlebar *titleIcon = new DTitlebar();
    titleIcon->setFrameStyle(QFrame::NoFrame); //无边框
    titleIcon->setBackgroundTransparent(true); //透明
    titleIcon->setMenuVisible(false);
    titleIcon->setTitle(tr("Select your language and add input methods"));
    titleIcon->setIcon(QIcon(":/img/title_img.png"));
    wrapperLayout->addWidget(titleIcon);

    auto *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(20, 20, 20, 10);
    mainLayout->setSpacing(10);
    wrapperLayout->addLayout(mainLayout);

    m_contentLayout = new QHBoxLayout();
    mainLayout->addLayout(m_contentLayout);

    auto *left = new DFrame(this);
    left->setFixedWidth(290);
    
    m_contentLayout->addWidget(left);

    m_leftLayout = new QVBoxLayout();
    m_leftLayout->setSpacing(10);
    m_leftLayout->setContentsMargins(10, 10, 10, 10);
    left->setLayout(m_leftLayout);

    m_searchEdit = new Dtk::Widget::DSearchEdit();
    m_leftLayout->addWidget(m_searchEdit);

    line = new DHorizontalLine(this);
    m_leftLayout->addWidget(line);

    m_availIMList = new DListView(this);
    m_availIMList->setModel(m_config->availIMModel());
    m_availIMList->setBackgroundType(DStyledItemDelegate::BackgroundType::RoundedBackground);
    m_availIMList->setFocusPolicy(Qt::NoFocus);
    m_availIMList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_leftLayout->addWidget(m_availIMList);

    m_rightLayout = new QVBoxLayout();
    m_rightLayout->setSpacing(10);
    m_contentLayout->addLayout(m_rightLayout);

    DFrame *rightListFrame = new DFrame(this);
    rightListFrame->setFixedWidth(350);
    QVBoxLayout *rightListFrameLayout = new QVBoxLayout();
    rightListFrame->setLayout(rightListFrameLayout);
    m_rightLayout->addWidget(rightListFrame);

    m_childIMList = new DListView(this);
    m_childIMList->setModel(m_config->availIMModel());
    m_childIMList->setBackgroundType(DStyledItemDelegate::BackgroundType::RoundedBackground);
    m_childIMList->setFocusPolicy(Qt::NoFocus);
    m_childIMList->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    m_childIMList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rightListFrameLayout->addWidget(m_childIMList);

    DFrame *laFrame = new DFrame(this);
    laFrame->setFixedSize(350, 130);
    QVBoxLayout *laFrameLayout = new QVBoxLayout();
    laFrame->setLayout(laFrameLayout);
    m_rightLayout->addWidget(laFrame);

    m_laSelector = fcitx::addim::LayoutSelector::selectLayout(this,
                                                              m_dbus,
                                                              _("Select default layout"),
                                                              "cn");
    laFrameLayout->addWidget(m_laSelector);

    auto *findMoreLayout =  new QHBoxLayout(this);
    mainLayout->addLayout(findMoreLayout);

    m_findMoreLabel = new DCommandLinkButton(tr("Find more in App Store"), this);
    m_findMoreLabel->setAccessibleName("Find more in App Store");
    findMoreLayout->addWidget(m_findMoreLabel);
    findMoreLayout->addStretch(1);

    mainLayout->addSpacing(9);

    m_buttonTuple = new ButtonTuple(ButtonTuple::Save, this);
    mainLayout->addWidget(m_buttonTuple);

    QPushButton *cancel = m_buttonTuple->leftButton();
    cancel->setText(tr("Cancel"));
    cancel->setObjectName("Cancel");
    QPushButton *ok = m_buttonTuple->rightButton();
    ok->setText(tr("Add"));
    ok->setEnabled(false);

    connect(m_config, &IMConfig::changed, this, &IMPage::changed);

    connect(m_searchEdit, &Dtk::Widget::DSearchEdit::textChanged, m_config->availIMModel(), &IMProxyModel::setFilterText);

    connect(m_availIMList->model(), &QAbstractItemModel::layoutChanged, this, [this]() {
        m_availIMList->setCurrentIndex(m_availIMList->model()->index(0, 0));
    });
    connect(m_availIMList->selectionModel(), &QItemSelectionModel::currentChanged, this, &IMPage::availIMCurrentChanged, Qt::QueuedConnection);
    connect(m_childIMList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &IMPage::childIMSelectionChanged);

    connect(m_findMoreLabel, &DCommandLinkButton::clicked, this, &IMPage::clickedFindMoreButton);

    connect(cancel, &QPushButton::clicked, this, &IMPage::clickedCloseButton);
    connect(ok, &QPushButton::clicked, this, &IMPage::clickedAddButton);

    m_availIMList->setCurrentIndex(m_availIMList->model()->index(0, 0));
}

IMPage::~IMPage()
{
    m_config->availIMModel()->setFilterText("");
}

void IMPage::save()
{
    QItemSelectionModel *selections = m_childIMList->selectionModel();
    QModelIndexList selected = selections->selectedIndexes();

    m_config->addIMs(selected);

    m_config->save();
}

void IMPage::load() {
    m_config->load();
}

void IMPage::defaults() { }

static QString getLayoutString(const QString &uniqName, const QString &langCode)
{
    QString layoutStr = "cn";
    if (uniqName.startsWith("keyboard-")) {
        // layout, name likes keyboard-xx-xxx, such as keyboard-cn-mon_trad_manchu
        auto dashPos = uniqName.indexOf("-");
        if (dashPos >= 0) {
            layoutStr = uniqName.mid(dashPos + 1);
        } else {
            qInfo("unexpected uniqName=%s", uniqName.toStdString().c_str());
            assert(0);
        }
    } else {
        // uniq name is not layout name, just input method name,
        // such as pinyin, wbx, wbpy, shuangpin
        // maybe we should use languageCode info in FcitxQtInputMethodEntry
        if (langCode == QString("zh_CN")) {
            layoutStr = "cn";
        }
    }
    return layoutStr;
}

void IMPage::availIMCurrentChanged(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    auto firstChild = index.model()->index(0, 0, index);
    if (!firstChild.isValid()) {
        return;
    }

    // workaround: index 与 child 的 parent 不同
    m_childIMList->setRootIndex(firstChild.parent());
    m_childIMList->selectionModel()->setCurrentIndex(firstChild, QItemSelectionModel::ClearAndSelect);
}

void IMPage::childIMSelectionChanged(const QItemSelection &selection)
{
    m_buttonTuple->rightButton()->setEnabled(!selection.isEmpty());

    for (auto &i : selection.indexes()) {
        QString uniqueName = i.data(FcitxIMUniqueNameRole).toString();
        QString langCode = i.data(FcitxLanguageRole).toString();
        m_laSelector->setLayout(getLayoutString(uniqueName, langCode), "");
    }
}

void IMPage::clickedFindMoreButton()
{
    DDBusSender()
            .service("com.home.appstore.client")
            .interface("com.home.appstore.client")
            .path("/com/home/appstore/client")
            .method("newInstence")
            .call();
}

void IMPage::clickedCloseButton()
{
    close();
    deleteLater();
}

void IMPage::clickedAddButton()
{
    save();

    close();
    deleteLater();
}

}
}
