// SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#include "glo.h"
#include "imelog.h"
#include "impage.h"
#include "categoryhelper.h"
#include "addimmodel.h"
#include "ui_impage.h"
#include "layoutselector.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QStyledItemDelegate>
#include <fcitxqtcontrollerproxy.h>
#include <DDialog>
#include <DGuiApplicationHelper>
#include <DDBusSender>
DWIDGET_USE_NAMESPACE
using namespace Dtk::Gui;

namespace fcitx {
namespace addim {

class IMDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit IMDelegate(QObject *parent = 0);
    virtual ~IMDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

IMDelegate::IMDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

IMDelegate::~IMDelegate() {}

void IMDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.data(FcitxRowTypeRole).toInt() == IMType) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    paintCategoryHeader(painter, option, index);
}

QSize IMDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.data(FcitxRowTypeRole).toInt() == IMType) {
        return QSize(0, 0);
    } else {
        return categoryHeaderSizeHint();
    }
}

class IMListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit IMListDelegate(QObject* parent = 0);
    virtual ~IMListDelegate();

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

IMListDelegate::IMListDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

IMListDelegate::~IMListDelegate() {}

void IMListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    bool useIM = index.data(FcitxUseIMRole).toBool();

    QFont font(QApplication::font());
    font.setFamily("SourceHanSansSC-Medium");
    int point_size = font.pointSize();
    font.setPixelSize(point_size + 2);

    QRectF rect;
    rect.setX(option.rect.x());
    rect.setY(option.rect.y());
    rect.setWidth(option.rect.width() - 1);
    rect.setHeight(option.rect.height() - 1);

    const qreal radius = 7;
    QPainterPath path;
    path.moveTo(rect.topRight() - QPointF(radius, 0));
    path.lineTo(rect.topLeft() + QPointF(radius, 0));
    path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, radius));
    path.lineTo(rect.bottomLeft() + QPointF(0, -radius));
    path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(radius, 0));
    path.lineTo(rect.bottomRight() - QPointF(radius, 0));
    path.quadTo(rect.bottomRight(), rect.bottomRight() + QPointF(0, -radius));
    path.lineTo(rect.topRight() + QPointF(0, radius));
    path.quadTo(rect.topRight(), rect.topRight() + QPointF(-radius, -0));

    if (useIM) {
        painter->setPen(QPen(Qt::gray));
        painter->fillPath(path, DGuiApplicationHelper::instance()->applicationPalette().base());
    } else {
        int currentIMIndex = getCurrentIMViewIndex();
        if (option.state.testFlag(QStyle::State_Selected) || (index.row() == currentIMIndex && currentIMIndex != -1)) {
            painter->setPen(QPen(Qt::white));
            painter->fillPath(path, DGuiApplicationHelper::instance()->applicationPalette().highlight());
        } else if (option.state.testFlag(QStyle::State_MouseOver)) {
            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
                painter->setPen(QPen(Qt::white));
            }
            else {
                painter->setPen(QPen(Qt::black));
            }
            painter->fillPath(path, DGuiApplicationHelper::instance()->applicationPalette().light());
        } else {
            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
                painter->setPen(QPen(Qt::white));
            }
            else {
                painter->setPen(QPen(Qt::black));
            }
            painter->fillPath(path, DGuiApplicationHelper::instance()->applicationPalette().base());
        }
    }

    painter->setFont(font);

    auto value = index.data(Qt::DisplayRole);
    if (value.isValid()) {
        QRect textRect(option.rect);
        textRect.setTop(textRect.top() + 8);
        textRect.setLeft(textRect.left() + 8);
        textRect.setHeight(20);
        textRect.setRight(textRect.right() - 8);

        QString text = value.toString();
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
    }

    painter->restore();
}

IMPage::IMPage(DBusProvider* dbus, IMConfig* config, QWidget* parent)
    : QWidget(parent)
    , ui_(std::make_unique<Ui::IMPage>())
    , m_dbus(dbus),
    m_config(config)
{
    ui_->setupUi(this);

    connect(m_config, &IMConfig::changed, this, &IMPage::changed);
    ui_->availIMView->setItemDelegate(new IMDelegate);
    ui_->availIMView->setModel(m_config->availIMModel());

    QFont font(QApplication::font());
    font.setFamily("SourceHanSansSC-Medium");
    int point_size = font.pointSize();
    osaLogInfo(LOG_TEST_NAME, LOG_TEST_NUM, "NOTICE: point_size [%d]\n", point_size);
    font.setPixelSize(point_size + 2);
    ui_->availIMView->setFont(font);
    ui_->currentIMView->setItemDelegate(new IMListDelegate);
    ui_->currentIMView->setModel(m_config->currentFilteredIMModel());
    ui_->currentIMView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_SearchEdit = new Dtk::Widget::DSearchEdit();
    m_SearchEdit->setFixedHeight(270);
    m_SearchEdit->setFixedHeight(36);
    m_SearchEdit->lineEdit()->setMaxLength(256);

    ui_->layout_search_edit->addWidget(m_SearchEdit);

    ui_->layout_leftveiw_in->setParent(NULL);
    m_wleftview = new BaseWidget("");
    m_wleftview->setFixedSize(290, 457);
    m_wleftview->setMinimumHeight(457);
    m_wleftview->setMaximumHeight(457);
    m_wleftview->setMinimumWidth(290);
    m_wleftview->setMaximumWidth(290);
    m_wleftview->setContentsMargins(0, 0, 0, 0);
    m_wleftview->setLayout(ui_->layout_leftveiw_in);
    ui_->layout_leftview->addWidget(m_wleftview);

    ui_->layout_middleview_in->setParent(NULL);
    ui_->layout_middleview->addLayout(ui_->layout_middleview_in);

    ui_->layout_r_up_view->setParent(NULL);
    m_wrightupview = new BaseWidget("");
    m_wrightupview->setFixedSize(350, 318);
    m_wrightupview->setMinimumHeight(318);
    m_wrightupview->setMaximumHeight(318);
    m_wrightupview->setMinimumWidth(350);
    m_wrightupview->setMaximumWidth(350);
    m_wrightupview->setContentsMargins(0, 0, 0, 0);
    m_wrightupview->setLayout(ui_->layout_r_up_view);
    ui_->layout_rightview->addWidget(m_wrightupview);

    ui_->layout_r_down_view->setParent(NULL);
    m_wrightdownview = new BaseWidget("");
    m_wrightdownview->setFixedSize(350, 129);
    m_wrightdownview->setMinimumHeight(129);
    m_wrightdownview->setMaximumHeight(129);
    m_wrightdownview->setMinimumWidth(350);
    m_wrightdownview->setMaximumWidth(350);
    m_wrightdownview->setContentsMargins(0, 0, 0, 0);
    m_wrightdownview->setLayout(ui_->layout_r_down_view);
    ui_->layout_rightview->addWidget(m_wrightdownview);

    QFont label_font(QApplication::font());
    label_font.setPixelSize(12);
    ui_->label_store_download->setText(tr("Find more in App Store"));
    ui_->label_store_download->setFont(label_font);
    ui_->label_store_download->setStyleSheet("color:rgb(36,80,255)");
    ui_->label_store_download->installEventFilter(this);

    QPalette palette = ui_->line->palette();
    QColor outlineColor = QColor(0, 0, 0);
    outlineColor.setAlphaF(0.15);
    palette.setColor(QPalette::Dark, outlineColor);
    ui_->line->setPalette(palette);

    ui_->pb_close->setText(tr("Cancel"));
    ui_->pb_close->setFont(font);
    QColor color;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        color = QColor("#FFFFFF");
    }
    palette = ui_->pb_close->palette();
    palette.setColor(QPalette::ButtonText, color);
    ui_->pb_close->setPalette(palette);
    
    ui_->pb_add->setText(tr("Add"));
    ui_->pb_add->setFont(font);
    ui_->pb_add->setStyleSheet("color:rgb(0,129,255)");
    ui_->pb_add->setEnabled(false);

    connect(m_SearchEdit, &Dtk::Widget::DSearchEdit::focusChanged, this, [this]() {
            m_wleftview->update();
        });

    connect(m_SearchEdit, &Dtk::Widget::DSearchEdit::textChanged, this,
        [this](const QString& text) {
            if (!text.isEmpty()) {
                m_config->clearCurrentIMEntries();
            }
            m_config->availIMModel()->setFilterText(text);

            int count = 0;
            int useCount = 0;
            QModelIndex availIndex = this->ui_->availIMView->model()->index(0, 0);
            count = addIM(availIndex, text);
            setSelectCategoryRow(0);

            useCount = m_config->currentUseIMEntries().count();
            if (count > useCount) {
                QModelIndex currentIndex = this->ui_->currentIMView->model()->index(useCount, 0);
                this->ui_->currentIMView->setCurrentIndex(currentIndex);
                setCurrentIMViewIndex(useCount);
            }

        });

    connect(ui_->availIMView->selectionModel(),
            &QItemSelectionModel::currentChanged, this,
            &IMPage::availIMSelectionChanged);
    connect(ui_->currentIMView->selectionModel(),
            &QItemSelectionModel::currentChanged, this,
            &IMPage::currentIMCurrentChanged);

    connect(m_config, &IMConfig::imListChanged, this,
            &IMPage::currentIMCurrentChanged);
    connect(m_config, &IMConfig::imListChanged, this,
            &IMPage::availIMSelectionChanged);
//    connect(ui_->defaultLayoutButton, &QPushButton::clicked, this,
//            &IMPage::selectDefaultLayout);
    connect(ui_->availIMView, &QListView::clicked, this, &IMPage::clickAvailIM);
    connect(ui_->currentIMView, &QListView::clicked, this, &IMPage::clickCurrentIM);

    connect(ui_->pb_close, &QPushButton::clicked, this, &IMPage::clickedCloseButton);
    connect(ui_->pb_add, &QPushButton::clicked, this, &IMPage::clickedAddButton);

    currentIMCurrentChanged();
    availIMSelectionChanged();

    setSelectCategoryRow(0);
    QModelIndex availIndex = this->ui_->availIMView->model()->index(0, 0);
    addIM(availIndex);

    m_laSelector = fcitx::addim::LayoutSelector::selectLayout(
                this, m_dbus, _("Select default layout"), "cn");
    ui_->layout_r_down_view->addWidget(m_laSelector);

	int count;
	int useCount;
	count    = m_config->currentIMEntries().count();
	useCount = m_config->currentUseIMEntries().count();
	if (count > useCount) {
		QModelIndex currentIndex = this->ui_->currentIMView->model()->index(useCount, 0);
		this->ui_->currentIMView->setCurrentIndex(currentIndex);
		clickCurrentIM(currentIndex);
	}
	osaLogInfo(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "<==== count [%d], useCount [%d]\n", count, useCount);
}

IMPage::~IMPage() {
    m_config->availIMModel()->setFilterText("");
}

void IMPage::save() {
    checkDefaultLayout();

    QItemSelectionModel* selections = ui_->currentIMView->selectionModel();
    QModelIndexList selected        = selections->selectedIndexes();

    foreach (QModelIndex index, selected) {
        m_config->saveSelectedIM(index.row());
    }

    if (selected.count() <= 0) {
        int currentIMIndex;
        currentIMIndex = getCurrentIMViewIndex();
        m_config->saveSelectedIM(currentIMIndex);
    }
}

void IMPage::load() {
    m_config->load();
}

void IMPage::defaults() {}

void IMPage::availIMSelectionChanged() {
}

void IMPage::currentIMCurrentChanged() {
}

void IMPage::selectCurrentIM(const QModelIndex &index) {
    ui_->currentIMView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
}

static QString getLayoutString(const fcitx::FcitxQtInputMethodEntry &entry)
{
    QString layoutStr = "cn";
    QString uniqName = entry.uniqueName();
    if (uniqName.startsWith("keyboard-")){
        // layout, name likes keyboard-xx-xxx, such as keyboard-cn-mon_trad_manchu
        auto dashPos = uniqName.indexOf("-");
        if (dashPos >= 0) {
            layoutStr = uniqName.mid(dashPos+1);
        }
        else{
            osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "unexpected uniqName=%s", uniqName.toStdString().c_str());
            assert(0);
        }
    }
    else{
        // uniq name is not layout name, just input method name,
        // such as pinyin, wbx, wbpy, shuangpin
        // maybe we should use languageCode info in FcitxQtInputMethodEntry
        if (entry.languageCode() == QString("zh_CN")){
            layoutStr = "cn";
        }
    }
    return layoutStr;
}

void IMPage::clickCurrentIM(const QModelIndex &index) {
    QItemSelectionModel* selections = ui_->currentIMView->selectionModel();
    QModelIndexList selected        = selections->selectedIndexes();

    bool existUsedIM = true;
    bool usedIM      = false;
    foreach (QModelIndex index, selected) {
        usedIM = index.data(FcitxUseIMRole).toBool();
        if (usedIM == false) {
            existUsedIM = usedIM;
        }
    }

    int currentIMIndex = index.row();
    int preCurrentIMIndex;
    preCurrentIMIndex = getCurrentIMViewIndex();
    setCurrentIMViewIndex(currentIMIndex);
    printf("m_currentIMIndex [%d]\n", currentIMIndex);

    if (existUsedIM == false) {
        ui_->pb_add->setEnabled(true);
    } else {
        ui_->pb_add->setEnabled(false);
    }

    QModelIndex preIndex = this->ui_->currentIMView->model()->index(preCurrentIMIndex, 0);
    ui_->currentIMView->update(preIndex);

    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "index=%d, count=%d\n", index.row(), m_config->currentIMEntries().count());
    assert(index.row() >= 0 && index.row() < m_config->currentIMEntries().count());
    if(index.row() < 0 || index.row() >= m_config->currentIMEntries().count()) {
        return ;
    }
    QString str = m_config->currentIMEntries().at(index.row()).key();

    for (auto &imEntry : m_config->allIms()) {
        if(imEntry.uniqueName() == str) {
            m_laSelector->setLayout(getLayoutString(imEntry), "");
        }
    }

}

void IMPage::clickAvailIM(const QModelIndex &index)
{
    int row_index = index.row();
    QString matchStr = m_SearchEdit->text();
    addIM(index, matchStr);

    setSelectCategoryRow(row_index);

    int count;
    int useCount;
    int viewItemCount;
    count = m_config->currentIMEntries().count();
    useCount = m_config->currentUseIMEntries().count();
    if (count > useCount) {
        QModelIndex currentIndex = this->ui_->currentIMView->model()->index(useCount, 0);
        this->ui_->currentIMView->setCurrentIndex(currentIndex);
        clickCurrentIM(currentIndex);
    }
}

void IMPage::selectDefaultLayout() {
}

void IMPage::checkDefaultLayout() {
    const auto &imEntries = m_config->currentIMEntries();
    if (imEntries.size() > 0 &&
        imEntries[0].key() !=
            QString("keyboard-%0").arg(m_config->defaultLayout()) &&
        imEntries[0].key().startsWith("keyboard-")) {
        auto layoutString = imEntries[0].key().mid(9);
    }
}

void IMPage::clickedCloseButton() {
    emit closeAddIMWindow();
}

void IMPage::clickedAddButton() {
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "====>\n");
    save();
    emit closeAddIMWindow();
    osaLogInfo(LOG_CFGTOOL_NAME, LOG_CFGTOOL_NUM, "<====\n");
}

int IMPage::addIM(const QModelIndex &index, QString matchStr) { return m_config->addSelectedIM(index, matchStr); }

void IMPage::moveUpIM() {
    QModelIndex curIndex = ui_->currentIMView->currentIndex();
    if (!curIndex.isValid() || curIndex.row() == 0) {
        return;
    }
    QModelIndex nextIndex =
        m_config->currentFilteredIMModel()->index(curIndex.row() - 1, 0);
    if (!nextIndex.isValid()) {
        return;
    }
    m_config->move(curIndex.row(), curIndex.row() - 1);
    currentIMCurrentChanged();
}

void IMPage::moveDownIM() {
    QModelIndex curIndex = ui_->currentIMView->currentIndex();
    if (!curIndex.isValid()) {
        return;
    }
    QModelIndex nextIndex = m_config->currentFilteredIMModel()->index(curIndex.row() + 1, 0);
    if (!nextIndex.isValid()) {
        return;
    }
    m_config->move(curIndex.row(), curIndex.row() + 1);
    currentIMCurrentChanged();
}

bool IMPage::eventFilter(QObject *watched, QEvent *event)
{
    if (ui_->label_store_download == watched) {
        if (event->type() == QEvent::MouseButtonPress) {
            DDBusSender().service("com.home.appstore.client")
                    .interface("com.home.appstore.client")
                    .path("/com/home/appstore/client")
                    .method("newInstence")
                    .call();
            return true;
        }
    }
    return false;
}

BaseWidget::BaseWidget(const QString& text, QWidget* parent, Qt::WindowFlags f)
	: QWidget(parent, f)
{
}

void BaseWidget::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    const int radius = 8;
    QRect paintRect = e->rect();
    QPainterPath path;
    path.moveTo(paintRect.bottomRight() - QPoint(0, radius));
    path.lineTo(paintRect.topRight() + QPoint(0, radius));
    path.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)), QSize(radius * 2, radius * 2)), 0, 90);
    path.lineTo(paintRect.topLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
    path.lineTo(paintRect.bottomLeft() - QPoint(0, radius));
    path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)), QSize(radius * 2, radius * 2)), 180, 90);
    path.lineTo(paintRect.bottomLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)), QSize(radius * 2, radius * 2)), 270, 90);
    painter.fillPath(path, DGuiApplicationHelper::instance()->applicationPalette().base());
}

}
}

#include "impage.moc"
