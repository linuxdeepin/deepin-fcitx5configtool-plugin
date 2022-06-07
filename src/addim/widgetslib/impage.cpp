/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "glo.h"
#include "imelog.h"
#include "impage.h"
#include "categoryhelper.h"
#include "addimmodel.h"
#include "ui_impage.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QStyledItemDelegate>
#include <fcitxqtcontrollerproxy.h>
#include <DDialog>
#include <DGuiApplicationHelper>
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

IMPage::IMPage(DBusProvider *dbus, IMConfig *config, QWidget *parent)
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
    BaseWidget* w_leftview = new BaseWidget("");
    w_leftview->setFixedSize(290, 457);
    w_leftview->setMinimumHeight(457);
    w_leftview->setMaximumHeight(457);
    w_leftview->setMinimumWidth(290);
    w_leftview->setMaximumWidth(290);
    w_leftview->setContentsMargins(0, 0, 0, 0);
    w_leftview->setLayout(ui_->layout_leftveiw_in);
    ui_->layout_leftview->addWidget(w_leftview);

    ui_->layout_middleview_in->setParent(NULL);
    ui_->layout_middleview->addLayout(ui_->layout_middleview_in);

    ui_->layout_r_up_view->setParent(NULL);
    BaseWidget* w_r_up_view = new BaseWidget("");
    w_r_up_view->setFixedSize(350, 318);
    w_r_up_view->setMinimumHeight(318);
    w_r_up_view->setMaximumHeight(318);
    w_r_up_view->setMinimumWidth(350);
    w_r_up_view->setMaximumWidth(350);
    w_r_up_view->setContentsMargins(0, 0, 0, 0);
    w_r_up_view->setLayout(ui_->layout_r_up_view);
    ui_->layout_rightview->addWidget(w_r_up_view);

    ui_->layout_r_down_view->setParent(NULL);
    BaseWidget* w_r_down_view = new BaseWidget("");
    w_r_down_view->setFixedSize(350, 129);
    w_r_down_view->setMinimumHeight(129);
    w_r_down_view->setMaximumHeight(129);
    w_r_down_view->setMinimumWidth(350);
    w_r_down_view->setMaximumWidth(350);
    w_r_down_view->setContentsMargins(0, 0, 0, 0);
    w_r_down_view->setLayout(ui_->layout_r_down_view);
    ui_->layout_rightview->addWidget(w_r_down_view);

    QFont label_font(QApplication::font());
    label_font.setPixelSize(12);
    ui_->label_store_download->setFont(label_font);
    ui_->label_store_download->setStyleSheet("color:rgb(36,80,255)");

    QPalette palette = ui_->line->palette();
    QColor outlineColor = QColor(0, 0, 0);
    outlineColor.setAlphaF(0.15);
    palette.setColor(QPalette::Dark, outlineColor);
    ui_->line->setPalette(palette);

    ui_->pb_add->setStyleSheet("color:rgb(0,129,255)");
    ui_->pb_add->setEnabled(false);

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
    connect(ui_->defaultLayoutButton, &QPushButton::clicked, this,
            &IMPage::selectDefaultLayout);
    connect(ui_->availIMView, &QListView::clicked, this, &IMPage::clickAvailIM);
    connect(ui_->currentIMView, &QListView::clicked, this, &IMPage::clickCurrentIM);

    connect(ui_->pb_close, &QPushButton::clicked, this, &IMPage::clickedCloseButton);
    connect(ui_->pb_add, &QPushButton::clicked, this, &IMPage::clickedAddButton);

    currentIMCurrentChanged();
    availIMSelectionChanged();

    setSelectCategoryRow(0);
    QModelIndex availIndex = this->ui_->availIMView->model()->index(0, 0);
    addIM(availIndex);

	int count;
	int useCount;
	int viewItemCount;
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
}

void IMPage::clickAvailIM(const QModelIndex &index)
{
    QString matchStr = m_SearchEdit->text();
    setCurrentIMViewIndex(-1);
    addIM(index, matchStr);
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
