/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "logging.h"
#include "addonselector.h"
#include "addonmodel.h"
#include "categoryhelper.h"
#include "configwidget.h"
#include "dbusprovider.h"
#include "model.h"
#include "ui_addonselector.h"
#include <KWidgetItemDelegate>
#include <QApplication>
#include <QCheckBox>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <fcitx-utils/i18n.h>
#include <fcitxqtcontrollerproxy.h>
#include <fcitxqtdbustypes.h>

constexpr int MARGIN = 5;

namespace fcitx {
namespace kcm {

class AddonDelegate : public KWidgetItemDelegate {
    Q_OBJECT

public:
    AddonDelegate(QAbstractItemView *listView, AddonSelector *parent);
    virtual ~AddonDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

Q_SIGNALS:
    void changed();
    void configCommitted(const QByteArray &addonName);

protected:
    QList<QWidget *> createItemWidgets(const QModelIndex &index) const override;
    void updateItemWidgets(const QList<QWidget *> &widgets,
                           const QStyleOptionViewItem &option,
                           const QPersistentModelIndex &index) const override;

private Q_SLOTS:
    void checkBoxClicked(bool state);
    void configureClicked();

private:
    QFont titleFont(const QFont &baseFont) const;
    int dependantLayoutValue(int value, int width, int totalWidth) const;

    QCheckBox *checkBox_;
    QToolButton *pushButton_;
    AddonSelector *parent_;
};

int AddonDelegate::dependantLayoutValue(int value, int width,
                                        int totalWidth) const {
    // qCDebug(KCM_FCITX5) << "dependantLayoutValue called with value:" << value << "width:" << width << "totalWidth:" << totalWidth;
    if (itemView()->layoutDirection() == Qt::LeftToRight) {
        // qCDebug(KCM_FCITX5) << "itemView()->layoutDirection() == Qt::LeftToRight";
        return value;
    }

    // qCDebug(KCM_FCITX5) << "itemView()->layoutDirection() == Qt::RightToLeft";
    return totalWidth - width - value;
}

QFont AddonDelegate::titleFont(const QFont &baseFont) const {
    // qCDebug(KCM_FCITX5) << "titleFont called with baseFont:" << baseFont;
    QFont retFont(baseFont);
    retFont.setBold(true);

    return retFont;
}

AddonDelegate::AddonDelegate(QAbstractItemView *listView, AddonSelector *parent)
    : KWidgetItemDelegate(listView, parent), checkBox_(new QCheckBox),
      pushButton_(new QToolButton), parent_(parent) {
    // qCDebug(KCM_FCITX5) << "AddonDelegate constructor called";
    pushButton_->setIcon(QIcon::fromTheme(
        "preferences-system-symbolic")); // only for getting size matters
}

AddonDelegate::~AddonDelegate() {
    // qCDebug(KCM_FCITX5) << "AddonDelegate destructor called";
    delete checkBox_;
    delete pushButton_;
}

void AddonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const {
    // qCDebug(KCM_FCITX5) << "AddonDelegate::paint called for index:" << index;
    if (!index.isValid()) {
        // qCWarning(KCM_FCITX5) << "Invalid index in AddonDelegate::paint";
        return;
    }

    if (index.data(RowTypeRole).toInt() == CategoryType) {
        paintCategoryHeader(painter, option, index);
        // qCDebug(KCM_FCITX5) << "Exiting AddonDelegate::paint (category header)";
        return;
    }

    int xOffset = 0;
    if (parent_->showAdvanced())
        xOffset = checkBox_->sizeHint().width();

    painter->save();

    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option,
                                         painter, 0);

    QRect contentsRect(
        dependantLayoutValue(MARGIN * 2 + option.rect.left() + xOffset,
                             option.rect.width() - MARGIN * 2 - xOffset,
                             option.rect.width()),
        MARGIN + option.rect.top(), option.rect.width() - MARGIN * 2 - xOffset,
        option.rect.height() - MARGIN * 2);

    int lessHorizontalSpace = MARGIN * 2 + pushButton_->sizeHint().width();

    contentsRect.setWidth(contentsRect.width() - lessHorizontalSpace);

    if (option.state & QStyle::State_Selected)
        painter->setPen(option.palette.highlightedText().color());

    if (itemView()->layoutDirection() == Qt::RightToLeft)
        contentsRect.translate(lessHorizontalSpace, 0);

    painter->save();

    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);
    painter->setFont(font);
    painter->drawText(
        contentsRect, Qt::AlignLeft | Qt::AlignTop,
        fmTitle.elidedText(
            index.model()->data(index, Qt::DisplayRole).toString(),
            Qt::ElideRight, contentsRect.width()));
    painter->restore();

    painter->drawText(contentsRect, Qt::AlignLeft | Qt::AlignBottom,
                      option.fontMetrics.elidedText(
                          index.model()->data(index, CommentRole).toString(),
                          Qt::ElideRight, contentsRect.width()));
    painter->restore();
    // qCDebug(KCM_FCITX5) << "Exiting AddonDelegate::paint";
}

QSize AddonDelegate::sizeHint(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const {
    // qCDebug(KCM_FCITX5) << "Entering AddonDelegate::sizeHint for index" << index.row();
    if (index.data(RowTypeRole).toInt() == CategoryType) {
        // qCDebug(KCM_FCITX5) << "Exiting AddonDelegate::sizeHint (category header)";
        return categoryHeaderSizeHint();
    }
    int i = 4;
    int j = 1;

    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);

    QRect titleBoundingRect = fmTitle.boundingRect(
        index.model()->data(index, Qt::DisplayRole).toString());

    QRect commentBoundingRect = option.fontMetrics.boundingRect(
        index.model()->data(index, CommentRole).toString());

    return QSize(
        titleBoundingRect.width() + 0 + MARGIN * i +
            pushButton_->sizeHint().width() * j,
        titleBoundingRect.height() +
            qMax(commentBoundingRect.height(), option.fontMetrics.height()) +
            MARGIN * 2);
}

QList<QWidget *>
AddonDelegate::createItemWidgets(const QModelIndex &index) const {
    // qCDebug(KCM_FCITX5) << "Entering AddonDelegate::createItemWidgets for index" << index.row();
    if (index.data(RowTypeRole).toInt() == CategoryType) {
        // qCDebug(KCM_FCITX5) << "Exiting AddonDelegate::createItemWidgets (category header)";
        return {};
    }
    QList<QWidget *> widgetList;

    QCheckBox *enabledCheckBox = new QCheckBox;
    connect(enabledCheckBox, &QCheckBox::clicked, this,
            &AddonDelegate::checkBoxClicked);

    QToolButton *configurePushButton = new QToolButton;
    configurePushButton->setIcon(
        QIcon::fromTheme("preferences-system-symbolic"));
    configurePushButton->setText(_("Configure"));
    connect(configurePushButton, &QToolButton::clicked, this,
            &AddonDelegate::configureClicked);

    setBlockedEventTypes(enabledCheckBox, QList<QEvent::Type>()
                                              << QEvent::MouseButtonPress
                                              << QEvent::MouseButtonRelease
                                              << QEvent::MouseButtonDblClick
                                              << QEvent::KeyPress
                                              << QEvent::KeyRelease);

    setBlockedEventTypes(configurePushButton, QList<QEvent::Type>()
                                                  << QEvent::MouseButtonPress
                                                  << QEvent::MouseButtonRelease
                                                  << QEvent::MouseButtonDblClick
                                                  << QEvent::KeyPress
                                                  << QEvent::KeyRelease);

    widgetList << enabledCheckBox << configurePushButton;

    // qCDebug(KCM_FCITX5) << "Exiting AddonDelegate::createItemWidgets";
    return widgetList;
}

void AddonDelegate::updateItemWidgets(const QList<QWidget *> &widgets,
                       const QStyleOptionViewItem &option,
                       const QPersistentModelIndex &index) const {
    // qCDebug(KCM_FCITX5) << "Entering AddonDelegate::updateItemWidgets for index" << index.row();
    if (index.data(RowTypeRole).toInt() == CategoryType) {
        // qCDebug(KCM_FCITX5) << "Exiting AddonDelegate::updateItemWidgets (category header)";
        return;
    }
    QCheckBox *checkBox = static_cast<QCheckBox *>(widgets[0]);
    checkBox->resize(checkBox->sizeHint());
    checkBox->move(dependantLayoutValue(MARGIN, checkBox->sizeHint().width(),
                                        option.rect.width()),
                   option.rect.height() / 2 -
                       checkBox->sizeHint().height() / 2);
    checkBox->setVisible(parent_->showAdvanced());

    QPushButton *configurePushButton = static_cast<QPushButton *>(widgets[1]);
    QSize configurePushButtonSizeHint = configurePushButton->sizeHint();
    configurePushButton->resize(configurePushButtonSizeHint);
    configurePushButton->move(
        dependantLayoutValue(
            option.rect.width() - MARGIN - configurePushButtonSizeHint.width(),
            configurePushButtonSizeHint.width(), option.rect.width()),
        option.rect.height() / 2 - configurePushButtonSizeHint.height() / 2);

    if (!index.isValid() || !index.internalPointer()) {
        // qCDebug(KCM_FCITX5) << "Invalid index or internal pointer, hiding widgets.";
        checkBox->setVisible(false);
        configurePushButton->setVisible(false);
    } else {
        // qCDebug(KCM_FCITX5) << "Valid index and internal pointer, showing widgets.";
        checkBox->setChecked(
            index.model()->data(index, Qt::CheckStateRole).toBool());
        configurePushButton->setEnabled(
            index.model()->data(index, Qt::CheckStateRole).toBool());
        configurePushButton->setVisible(
            index.model()->data(index, ConfigurableRole).toBool());
    }
    // qCDebug(KCM_FCITX5) << "Exiting AddonDelegate::updateItemWidgets";
}

void AddonDelegate::checkBoxClicked(bool state) {
    qCDebug(KCM_FCITX5) << "AddonDelegate::checkBoxClicked";
    if (!focusedIndex().isValid()) {
        qCWarning(KCM_FCITX5) << "Invalid focused index in checkBoxClicked";
        return;
    }
    const QModelIndex index = focusedIndex();

    const_cast<QAbstractItemModel *>(index.model())
        ->setData(index, state, Qt::CheckStateRole);
    qCDebug(KCM_FCITX5) << "Exiting AddonDelegate::checkBoxClicked";
}

void AddonDelegate::configureClicked() {
    qCDebug(KCM_FCITX5) << "Entering AddonDelegate::configureClicked";
    const QModelIndex index = focusedIndex();
    auto name = index.data(AddonNameRole).toString();
    // qCDebug(KCM_FCITX5) << "AddonDelegate::configureClicked called for addon:" << name;
    if (name.isEmpty()) {
        qCWarning(KCM_FCITX5) << "Empty addon name in configureClicked";
        return;
    }
    auto addonName = index.data(Qt::DisplayRole).toString();
    QPointer<QDialog> dialog = ConfigWidget::configDialog(
        parent_, parent_->dbus(), QString("fcitx://config/addon/%1").arg(name),
        addonName);
    dialog->exec();
    delete dialog;
    qCDebug(KCM_FCITX5) << "Exiting AddonDelegate::configureClicked";
}

AddonSelector::AddonSelector(QWidget *parent, DBusProvider *dbus)
    : QWidget(parent), dbus_(dbus), addonModel_(new AddonModel(this)),
      proxyModel_(new AddonProxyModel(this)),

      ui_(std::make_unique<Ui::AddonSelector>()) {
    qCDebug(KCM_FCITX5) << "AddonSelector constructor called";
    ui_->setupUi(this);

    connect(dbus_, &DBusProvider::availabilityChanged, this,
            &AddonSelector::availabilityChanged);

    proxyModel_->setSourceModel(addonModel_);
    ui_->listView->setModel(proxyModel_);
    connect(proxyModel_, &QAbstractItemModel::layoutChanged, ui_->listView,
            &QTreeView::expandAll);
    connect(addonModel_, &AddonModel::changed, this,
            [this](const QString &addon, bool enabled) {
                if (!enabled) {
                    if (!reverseDependencies_.value(addon).empty() ||
                        !reverseOptionalDependencies_.value(addon).empty()) {
                        QMetaObject::invokeMethod(
                            this, [this, addon]() { warnAddonDisable(addon); },
                            Qt::QueuedConnection);
                    }
                }
                Q_EMIT changed();
            });

    delegate_ = new AddonDelegate(ui_->listView, this);
    ui_->listView->setItemDelegate(delegate_);
    ui_->listView->viewport()->setAttribute(Qt::WA_Hover);

    connect(ui_->lineEdit, &QLineEdit::textChanged, proxyModel_,
            &AddonProxyModel::setFilterText);
    connect(ui_->advancedCheckbox, &QCheckBox::toggled, this, [this]() {
        if (showAdvanced()) {
            QMessageBox::warning(
                this, _("Advanced options"),
                _("The feature of enabling/disabling addons is only intended "
                  "for advanced users who understand the potential "
                  "implication. Fcitx needs to be restarted to make the "
                  "changes to enable/disable to take effect."));
        }
        proxyModel_->invalidate();
    });
    connect(addonModel_, &AddonProxyModel::dataChanged, this,
            [this]() { proxyModel_->invalidate(); });
    qCDebug(KCM_FCITX5) << "Exiting AddonSelector::AddonSelector";
}

AddonSelector::~AddonSelector() {
    // qCDebug(KCM_FCITX5) << "AddonSelector destructor called";
    delete delegate_;
}

void AddonSelector::load() {
    // qCDebug(KCM_FCITX5) << "AddonSelector::load called";
    availabilityChanged();
}

void AddonSelector::save() {
    // qCDebug(KCM_FCITX5) << "AddonSelector::save called";

    if (!dbus_->controller()) {
        qCWarning(KCM_FCITX5) << "DBus controller not available in save";
        return;
    }
    FcitxQtAddonStateList list;
    for (auto &enabled : addonModel_->enabledList()) {
        FcitxQtAddonState state;
        state.setUniqueName(enabled);
        state.setEnabled(true);
        list.append(state);
    }
    for (auto &disabled : addonModel_->disabledList()) {
        FcitxQtAddonState state;
        state.setUniqueName(disabled);
        state.setEnabled(false);
        list.append(state);
    }
    if (list.size()) {
        // qCInfo(KCM_FCITX5) << "Saving" << list.size() << "addon state changes.";
        dbus_->controller()->SetAddonsState(list);
        load();
    }
    // qCDebug(KCM_FCITX5) << "Exiting AddonSelector::save";
}

void AddonSelector::availabilityChanged() {
    qCDebug(KCM_FCITX5) << "AddonSelector::availabilityChanged called";
    if (!dbus_->controller()) {
        qCWarning(KCM_FCITX5) << "DBus controller not available in availabilityChanged";
        return;
    }

    auto call = dbus_->controller()->GetAddonsV2();
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            &AddonSelector::fetchAddonFinished);
    qCDebug(KCM_FCITX5) << "Exiting AddonSelector::availabilityChanged";
}

void AddonSelector::fetchAddonFinished(QDBusPendingCallWatcher *watcher) {
    qCDebug(KCM_FCITX5) << "AddonSelector::fetchAddonFinished called";

    watcher->deleteLater();
    if (watcher->isError()) {
        qCWarning(KCM_FCITX5) << "Error fetching addons:" << watcher->error().message();
        return;
    }
    QDBusPendingReply<FcitxQtAddonInfoV2List> reply(*watcher);
    nameToAddonMap_.clear();
    reverseDependencies_.clear();
    reverseOptionalDependencies_.clear();
    auto list = reply.value();
    for (const auto &addon : list) {
        nameToAddonMap_[addon.uniqueName()] = addon;
    }
    for (const auto &addon : list) {
        for (const auto &dep : addon.dependencies()) {
            if (!nameToAddonMap_.contains(dep)) {
                continue;
            }
            reverseDependencies_[dep].append(addon.uniqueName());
        }
        for (const auto &dep : addon.optionalDependencies()) {
            if (!nameToAddonMap_.contains(dep)) {
                continue;
            }
            reverseOptionalDependencies_[dep].append(addon.uniqueName());
        }
    }
    addonModel_->setAddons(reply.value());
    proxyModel_->sort(0);

    ui_->listView->expandAll();
    qCDebug(KCM_FCITX5) << "Exiting AddonSelector::fetchAddonFinished";
}

void AddonSelector::warnAddonDisable(const QString &addon) {
    qCDebug(KCM_FCITX5) << "AddonSelector::warnAddonDisable called for addon:" << addon;

    if (!nameToAddonMap_.contains(addon)) {
        qCWarning(KCM_FCITX5) << "Addon not found in nameToAddonMap:" << addon;
        return;
    }

    const auto &addonInfo = nameToAddonMap_[addon];
    QString depWarning, optDepWarning;
    QString sep{C_("Separator of a comma list", ", ")};
    if (auto deps = reverseDependencies_.value(addon); !deps.empty()) {
        // qCDebug(KCM_FCITX5) << "Addon has dependencies.";
        QStringList addonNames;
        for (const auto &dep : deps) {
            auto iter = nameToAddonMap_.find(dep);
            if (iter == nameToAddonMap_.end()) {
                continue;
            }
            addonNames << iter->name();
        }
        depWarning = QString(_("- Disable %1\n")).arg(addonNames.join(sep));
    }
    if (auto deps = reverseOptionalDependencies_.value(addon); !deps.empty()) {
        // qCDebug(KCM_FCITX5) << "Addon has optional dependencies.";
        QStringList addonNames;
        for (const auto &dep : deps) {
            auto iter = nameToAddonMap_.find(dep);
            if (iter == nameToAddonMap_.end()) {
                continue;
            }
            addonNames << iter->name();
        }
        optDepWarning = QString(_("- Disable some features in %1\n"))
                            .arg(addonNames.join(sep));
    }
    auto warning = QString(_("Disabling %1 will also:\n%2%3\nAre you sure you "
                             "want to disable it?"))
                       .arg(addonInfo.name(), depWarning, optDepWarning);
    if (QMessageBox::No ==
        QMessageBox::question(
            this, QString(_("Disable %1")).arg(addonInfo.name()), warning)) {
        qCDebug(KCM_FCITX5) << "User confirmed disabling addon:" << addon;
        addonModel_->setData(addonModel_->findAddon(addonInfo.uniqueName()),
                             true, Qt::CheckStateRole);
    }
    qCDebug(KCM_FCITX5) << "Exiting AddonSelector::warnAddonDisable";
}

QString AddonSelector::searchText() const { return ui_->lineEdit->text(); }

bool AddonSelector::showAdvanced() const {
    // qCDebug(KCM_FCITX5) << "AddonSelector::showAdvanced called";
    return ui_->advancedCheckbox->isChecked();
}

} // namespace kcm
} // namespace fcitx

#include "addonselector.moc"
