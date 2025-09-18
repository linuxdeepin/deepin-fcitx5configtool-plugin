/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "logging.h"
#include "impage.h"
#include "categoryhelper.h"
#include "configwidget.h"
#include "dbusprovider.h"
#include "layoutselector.h"
#include "model.h"
#include "ui_impage.h"
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QStyledItemDelegate>
#include <fcitxqtcontrollerproxy.h>

namespace fcitx {
namespace kcm {

bool isInFlatpak() {
    static bool inFlatpak = QFile::exists("/.flatpak-info");
    return inFlatpak;
}

class IMDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit IMDelegate(QObject *parent = 0);
    virtual ~IMDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

IMDelegate::IMDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

IMDelegate::~IMDelegate() {}

void IMDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const {
    // qCDebug(KCM_FCITX5) << "Painting IMDelegate";
    if (index.data(FcitxRowTypeRole).toInt() == IMType) {
        // qCDebug(KCM_FCITX5) << "Painting IMDelegate for IMType";
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    paintCategoryHeader(painter, option, index);
}

QSize IMDelegate::sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const {
    // qCDebug(KCM_FCITX5) << "Size hint for IMDelegate";
    if (index.data(FcitxRowTypeRole).toInt() == IMType) {
        // qCDebug(KCM_FCITX5) << "Size hint for IMDelegate for IMType";
        return QStyledItemDelegate::sizeHint(option, index);
    } else {
        // qCDebug(KCM_FCITX5) << "Size hint for IMDelegate for category header";
        return categoryHeaderSizeHint();
    }
}

IMPage::IMPage(DBusProvider *dbus, QWidget *parent)
    : QWidget(parent), ui_(std::make_unique<Ui::IMPage>()), dbus_(dbus),
      config_(new IMConfig(dbus, IMConfig::Tree, this)) {
    qCDebug(KCM_FCITX5) << "IMPage created with DBus";
    ui_->setupUi(this);
    ui_->availIMView->header()->setSortIndicator(0, Qt::AscendingOrder);
    ui_->addIMButton->setIcon(QIcon::fromTheme(
        "go-previous-symbolic", style()->standardIcon(QStyle::SP_ArrowLeft)));
    ui_->removeIMButton->setIcon(QIcon::fromTheme(
        "go-next-symbolic", style()->standardIcon(QStyle::SP_ArrowRight)));
    ui_->moveUpButton->setIcon(QIcon::fromTheme(
        "go-up-symbolic", style()->standardIcon(QStyle::SP_ArrowUp)));
    ui_->moveDownButton->setIcon(QIcon::fromTheme(
        "go-down-symbolic", style()->standardIcon(QStyle::SP_ArrowDown)));
    ui_->configureButton->setIcon(QIcon::fromTheme(
        "preferences-system-symbolic",
        style()->standardIcon(QStyle::SP_FileDialogDetailedView)));
    ui_->layoutButton->setIcon(
        QIcon::fromTheme("input-keyboard-symbolic",
                         style()->standardIcon(QStyle::SP_ComputerIcon)));
    ui_->addGroupButton->setIcon(QIcon::fromTheme(
        "list-add-symbolic",
        style()->standardIcon(QStyle::SP_FileDialogNewFolder)));
    ui_->deleteGroupButton->setIcon(QIcon::fromTheme(
        "list-remove-symbolic", style()->standardIcon(QStyle::SP_TrashIcon)));

    ui_->checkUpdateMessage->setVisible(false);
    connect(ui_->inputMethodGroupComboBox, &QComboBox::currentTextChanged, this,
            &IMPage::selectedGroupChanged);
    connect(config_, &IMConfig::changed, this, &IMPage::changed);
    connect(config_, &IMConfig::currentGroupChanged, this,
            [this](const QString &group) {
                ui_->inputMethodGroupComboBox->setCurrentText(group);
            });
    connect(config_, &IMConfig::groupsChanged, this,
            [this](const QStringList &groups) {
                ui_->inputMethodGroupComboBox->clear();
                for (const QString &group : groups) {
                    ui_->inputMethodGroupComboBox->addItem(group);
                }
                ui_->deleteGroupButton->setEnabled(groups.size() > 1);
            });
    connect(config_, &IMConfig::needUpdateChanged, ui_->checkUpdateMessage,
            &QWidget::setVisible);

    if (isInFlatpak()) {
        // qCDebug(KCM_FCITX5) << "In Flatpak, showing update message";
        ui_->checkUpdateMessage->setText(
            _("Found updates to fcitx installation. Do you want to restart "
              "Fcitx?"));
    } else {
        // qCDebug(KCM_FCITX5) << "Not in Flatpak, showing update message";
        auto refreshAction = new QAction(_("Update"));
        connect(refreshAction, &QAction::triggered, this,
                [this]() { config_->refresh(); });
        ui_->checkUpdateMessage->addAction(refreshAction);
    }

    auto restartAction = new QAction(_("Restart"));
    connect(restartAction, &QAction::triggered, this,
            [this]() { config_->restart(); });
    ui_->checkUpdateMessage->addAction(restartAction);

    ui_->availIMView->setItemDelegate(new IMDelegate);
    ui_->availIMView->setModel(config_->availIMModel());
    connect(config_->availIMModel(), &QAbstractItemModel::layoutChanged,
            ui_->availIMView, &QTreeView::expandAll);
    connect(config_, &IMConfig::imListChanged, ui_->availIMView,
            &QTreeView::expandAll);
    ui_->currentIMView->setModel(config_->currentIMModel());

    connect(ui_->filterTextEdit, &QLineEdit::textChanged, this,
            [this](const QString &text) {
                config_->availIMModel()->setFilterText(text);
                ui_->onlyCurrentLanguageCheckBox->setVisible(text.isEmpty());
            });
    connect(ui_->onlyCurrentLanguageCheckBox, &QCheckBox::toggled,
            config_->availIMModel(), &IMProxyModel::setShowOnlyCurrentLanguage);

    connect(ui_->availIMView->selectionModel(),
            &QItemSelectionModel::currentChanged, this,
            &IMPage::availIMSelectionChanged);
    connect(ui_->currentIMView->selectionModel(),
            &QItemSelectionModel::currentChanged, this,
            &IMPage::currentIMCurrentChanged);
    connect(config_, &IMConfig::imListChanged, this,
            &IMPage::currentIMCurrentChanged);
    connect(config_, &IMConfig::imListChanged, this,
            &IMPage::availIMSelectionChanged);
    connect(ui_->addIMButton, &QPushButton::clicked, this, &IMPage::clickAddIM);
    connect(ui_->removeIMButton, &QPushButton::clicked, this,
            &IMPage::clickRemoveIM);
    connect(ui_->moveUpButton, &QPushButton::clicked, this, &IMPage::moveUpIM);
    connect(ui_->moveDownButton, &QPushButton::clicked, this,
            &IMPage::moveDownIM);
    connect(ui_->configureButton, &QPushButton::clicked, this,
            &IMPage::configureIM);
    connect(ui_->addGroupButton, &QPushButton::clicked, this,
            &IMPage::addGroup);
    connect(ui_->deleteGroupButton, &QPushButton::clicked, this,
            &IMPage::deleteGroup);
    connect(ui_->defaultLayoutButton, &QPushButton::clicked, this,
            &IMPage::selectDefaultLayout);
    connect(ui_->layoutButton, &QPushButton::clicked, this,
            &IMPage::selectLayout);
    connect(ui_->availIMView, &QTreeView::doubleClicked, this,
            &IMPage::doubleClickAvailIM);
    connect(ui_->currentIMView, &QListView::doubleClicked, this,
            &IMPage::doubleClickCurrentIM);

    currentIMCurrentChanged();
    availIMSelectionChanged();
    qCDebug(KCM_FCITX5) << "Exiting IMPage constructor";
}

IMPage::~IMPage() {
    // qCDebug(KCM_FCITX5) << "IMPage destroyed";
}

void IMPage::save() {
    // qCDebug(KCM_FCITX5) << "Saving input method";
    checkDefaultLayout();
    config_->save();
}

void IMPage::load() {
    // qCDebug(KCM_FCITX5) << "Loading input method configuration";
    config_->load();
}

void IMPage::defaults() {}

void IMPage::selectedGroupChanged() {
    qCDebug(KCM_FCITX5) << "Selected group";
    if (config_->currentGroup() ==
        ui_->inputMethodGroupComboBox->currentText()) {
        qDebug() << "Group unchanged";
        return;
    }
    if (!config_->currentGroup().isEmpty() && config_->needSave()) {
        // qCDebug(KCM_FCITX5) << "Current group changed and need save.";
        if (QMessageBox::No ==
            QMessageBox::question(this, _("Current group changed"),
                                  _("Do you want to change group? Changes to "
                                    "current group will be lost!"))) {
            ui_->inputMethodGroupComboBox->setCurrentText(
                config_->currentGroup());
            qCDebug(KCM_FCITX5) << "User cancelled group change.";
            return;
        }
    }

    config_->setCurrentGroup(ui_->inputMethodGroupComboBox->currentText());
}

void IMPage::availIMSelectionChanged() {
    // qCDebug(KCM_FCITX5) << "Entering availIMSelectionChanged";
    if (!ui_->availIMView->currentIndex().isValid())
        ui_->addIMButton->setEnabled(false);
    else
        ui_->addIMButton->setEnabled(true);
    // qCDebug(KCM_FCITX5) << "Exiting availIMSelectionChanged, addIMButton enabled:" << ui_->addIMButton->isEnabled();
}

void IMPage::currentIMCurrentChanged() {
    qCDebug(KCM_FCITX5) << "Entering currentIMCurrentChanged";
    if (!ui_->currentIMView->currentIndex().isValid()) {
        qCDebug(KCM_FCITX5) << "No current IM selected, disabling buttons.";
        ui_->removeIMButton->setEnabled(false);
        ui_->moveUpButton->setEnabled(false);
        ui_->moveDownButton->setEnabled(false);
        ui_->configureButton->setEnabled(false);
        ui_->layoutButton->setEnabled(false);
    } else {
        qCDebug(KCM_FCITX5) << "Current IM selection changed, updating button states.";
        if (ui_->currentIMView->currentIndex().row() == 0)
            ui_->moveUpButton->setEnabled(false);
        else
            ui_->moveUpButton->setEnabled(true);
        if (ui_->currentIMView->currentIndex().row() ==
            config_->currentIMModel()->rowCount() - 1) {
            ui_->moveDownButton->setEnabled(false);
        } else {
            ui_->moveDownButton->setEnabled(true);
        }
        ui_->removeIMButton->setEnabled(true);
        ui_->configureButton->setEnabled(
            config_->currentIMModel()
                ->data(ui_->currentIMView->currentIndex(),
                       FcitxIMConfigurableRole)
                .toBool());
        ui_->layoutButton->setEnabled(
            !config_->currentIMModel()
                 ->data(ui_->currentIMView->currentIndex(),
                        FcitxIMUniqueNameRole)
                 .toString()
                 .startsWith("keyboard-"));
    }
    qCDebug(KCM_FCITX5) << "Exiting currentIMCurrentChanged";
}

void IMPage::selectCurrentIM(const QModelIndex &index) {
    qCDebug(KCM_FCITX5) << "Selecting current IM at index" << index;
    ui_->currentIMView->selectionModel()->setCurrentIndex(
        index, QItemSelectionModel::ClearAndSelect);
}

void IMPage::doubleClickCurrentIM(const QModelIndex &index) { removeIM(index); }

void IMPage::doubleClickAvailIM(const QModelIndex &index) { addIM(index); }

void IMPage::selectDefaultLayout() {
    qCDebug(KCM_FCITX5) << "Selecting default layout.";
    auto defaultLayout = config_->defaultLayout();
    auto dashPos = defaultLayout.indexOf("-");
    QString layout, variant;
    if (dashPos >= 0) {
        // qCDebug(KCM_FCITX5) << "Default layout has variant.";
        variant = defaultLayout.mid(dashPos + 1);
        layout = defaultLayout.left(dashPos);
    } else {
        // qCDebug(KCM_FCITX5) << "Default layout has no variant.";
        layout = defaultLayout;
    }
    bool ok = false;
    auto result = LayoutSelector::selectLayout(
        this, dbus_, _("Select default layout"), layout, variant, &ok);
    if (!ok) {
        // qCDebug(KCM_FCITX5) << "Layout selection cancelled.";
        return;
    }
    if (result.second.isEmpty()) {
        // qCDebug(KCM_FCITX5) << "Setting default layout to:" << result.first;
        config_->setDefaultLayout(result.first);
    } else {
        // qCDebug(KCM_FCITX5) << "Setting default layout to:" << result.first << "-" << result.second;
        config_->setDefaultLayout(
            QString("%0-%1").arg(result.first, result.second));
    }

    auto imname = QString("keyboard-%0").arg(config_->defaultLayout());
    if (config_->imEntries().empty() ||
        config_->imEntries().front().key() != imname) {
        // qCDebug(KCM_FCITX5) << "Default layout and first IM do not match, prompting user.";
        auto result = QMessageBox::question(
            this, _("Change Input method to match layout selection."),
            _("Your currently configured input method does not match your "
              "selected layout, do you want to add the corresponding input "
              "method for the layout?"),
            QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
            QMessageBox::Yes);
        if (result == QMessageBox::Yes) {
            // qCDebug(KCM_FCITX5) << "User chose to add matching keyboard layout.";
            FcitxQtStringKeyValue imEntry;
            int i = 0;
            auto imEntries = config_->imEntries();
            for (; i < imEntries.size(); i++) {
                if (imEntries[i].key() == imname) {
                    imEntry = imEntries[i];
                    imEntries.removeAt(i);
                    break;
                }
            }
            if (i == imEntries.size()) {
                imEntry.setKey(imname);
            }
            imEntries.push_front(imEntry);
            config_->setIMEntries(imEntries);
        }
    }
}

void IMPage::selectLayout() {
    qCDebug(KCM_FCITX5) << "Selecting layout for current IM.";
    QModelIndex curIndex = ui_->currentIMView->currentIndex();
    if (!curIndex.isValid()) {
        qCWarning(KCM_FCITX5) << "No current IM selected to set layout for.";
        return;
    }
    QString imName = curIndex.data(FcitxIMUniqueNameRole).toString();
    QString layoutString = curIndex.data(FcitxIMLayoutRole).toString();
    if (layoutString.isEmpty()) {
        // qCDebug(KCM_FCITX5) << "Layout string is empty, setting to default layout.";
        layoutString = config_->defaultLayout();
    }
    auto dashPos = layoutString.indexOf("-");
    QString layout, variant;
    if (dashPos >= 0) {
        // qCDebug(KCM_FCITX5) << "Layout string has variant.";
        variant = layoutString.mid(dashPos + 1);
        layout = layoutString.left(dashPos);
    } else {
        // qCDebug(KCM_FCITX5) << "Layout string has no variant.";
        layout = layoutString;
    }
    bool ok = false;
    auto result = LayoutSelector::selectLayout(this, dbus_, _("Select Layout"),
                                               layout, variant, &ok);
    if (!ok) {
        qCDebug(KCM_FCITX5) << "Layout selection cancelled, clearing layout for IM:" << imName;
        config_->setLayout(imName, "");
        return;
    }
    if (result.second.isEmpty()) {
        qCDebug(KCM_FCITX5) << "Setting layout for IM" << imName << "to:" << result.first;
        config_->setLayout(imName, result.first);
    } else {
        qCDebug(KCM_FCITX5) << "Setting layout for IM" << imName << "to:" << result.first << "-" << result.second;
        config_->setLayout(imName,
                           QString("%0-%1").arg(result.first, result.second));
    }
}

void IMPage::selectAvailIM(const QModelIndex &index) {
    qCDebug(KCM_FCITX5) << "Selecting available IM at index" << index;
    ui_->availIMView->selectionModel()->setCurrentIndex(
        config_->availIMModel()->mapFromSource(index),
        QItemSelectionModel::ClearAndSelect);
}

void IMPage::clickAddIM() { addIM(ui_->availIMView->currentIndex()); }

void IMPage::clickRemoveIM() { removeIM(ui_->currentIMView->currentIndex()); }

void IMPage::checkDefaultLayout() {
    qCDebug(KCM_FCITX5) << "Entering checkDefaultLayout";
    const auto &imEntries = config_->imEntries();
    if (imEntries.size() > 0 &&
        imEntries[0].key() !=
            QString("keyboard-%0").arg(config_->defaultLayout()) &&
        imEntries[0].key().startsWith("keyboard-")) {
        qCDebug(KCM_FCITX5) << "First IM is a keyboard layout but does not match default system layout, prompting user.";
        // Remove "keyboard-".
        auto layoutString = imEntries[0].key().mid(9);
        auto result = QMessageBox::question(
            this, _("Change System layout to match input method selection."),
            _("Your currently configured input method does not match your "
              "layout, do you want to change the layout setting?"),
            QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
            QMessageBox::Yes);
        if (result == QMessageBox::Yes) {
            qCDebug(KCM_FCITX5) << "User chose to change system layout to" << layoutString;
            config_->setDefaultLayout(layoutString);
        }
    }
    qCDebug(KCM_FCITX5) << "Exiting checkDefaultLayout";
}

void IMPage::addIM(const QModelIndex &index) {
    qCDebug(KCM_FCITX5) << "Adding input method";
    config_->addIM(index);
}

void IMPage::removeIM(const QModelIndex &index) {
    qCDebug(KCM_FCITX5) << "Removing input method";
    config_->removeIM(index);
}

void IMPage::configureIM() {
    qCDebug(KCM_FCITX5) << "Configure IM button clicked.";
    QModelIndex curIndex = ui_->currentIMView->currentIndex();
    if (!curIndex.isValid()) {
        qCWarning(KCM_FCITX5) << "No valid input method selected for configuration";
        return;
    }
    const QString uniqueName = curIndex.data(FcitxIMUniqueNameRole).toString();
    QPointer<QDialog> dialog = ConfigWidget::configDialog(
        this, dbus_, QString("fcitx://config/inputmethod/%1").arg(uniqueName),
        curIndex.data(Qt::DisplayRole).toString());
    dialog->exec();
    delete dialog;
    qCDebug(KCM_FCITX5) << "Config dialog for" << uniqueName << "closed.";
}

void IMPage::moveUpIM() {
    qCDebug(KCM_FCITX5) << "Move up IM button clicked.";
    QModelIndex curIndex = ui_->currentIMView->currentIndex();
    if (!curIndex.isValid() || curIndex.row() == 0) {
        qCWarning(KCM_FCITX5) << "Cannot move up, index invalid or already at top.";
        return;
    }
    QModelIndex nextIndex =
        config_->currentIMModel()->index(curIndex.row() - 1, 0);
    if (!nextIndex.isValid()) {
        qCWarning(KCM_FCITX5) << "Cannot move up, next index is invalid.";
        return;
    }
    config_->move(curIndex.row(), curIndex.row() - 1);
    currentIMCurrentChanged();
}

void IMPage::moveDownIM() {
    qCDebug(KCM_FCITX5) << "Move down IM button clicked.";
    QModelIndex curIndex = ui_->currentIMView->currentIndex();
    if (!curIndex.isValid()) {
        qCWarning(KCM_FCITX5) << "Cannot move down, index invalid.";
        return;
    }
    QModelIndex nextIndex =
        config_->currentIMModel()->index(curIndex.row() + 1, 0);
    if (!nextIndex.isValid()) {
        qCWarning(KCM_FCITX5) << "Cannot move down, already at bottom.";
        return;
    }
    config_->move(curIndex.row(), curIndex.row() + 1);
    currentIMCurrentChanged();
}

void IMPage::addGroup() {
    qCDebug(KCM_FCITX5) << "Adding new input method group";
    bool ok;
    QString name = QInputDialog::getText(this, _("New Group"), _("Group Name:"),
                                         QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        qCDebug(KCM_FCITX5) << "Adding new group:" << name
                           << "current groups:" << config_->groups();
        config_->addGroup(name);
    } else {
        qCDebug(KCM_FCITX5) << "Group creation canceled";
    }
}

void IMPage::deleteGroup() {
    QString group = ui_->inputMethodGroupComboBox->currentText();
    qCDebug(KCM_FCITX5) << "Deleting group:" << group
                       << "remaining groups:" << config_->groups();
    config_->deleteGroup(group);
}

} // namespace kcm
} // namespace fcitx

#include "impage.moc"
