/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "listoptionwidget.h"
#include "varianthelper.h"
#include "logging.h"

#include <QAbstractListModel>
#include <QDebug>
#include <QtGlobal>

namespace fcitx {
namespace kcm {

class ListOptionWidgetModel : public QAbstractListModel {
public:
    ListOptionWidgetModel(ListOptionWidget *parent)
        : QAbstractListModel(parent), parent_(parent) {}

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= values_.size()) {
            return QVariant();
        }
        const auto &value = values_.at(index.row());

        switch (role) {
        case Qt::DisplayRole:
            return parent_->prettify(parent_->subOption(), value);
        case Qt::UserRole:
            return value;
        }
        return QVariant();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) {
            return 0;
        }

        return values_.size();
    }

    void readValueFrom(const QVariantMap &map, const QString &path) {
        // qCDebug(KCM_FCITX5) << "Entering ListOptionWidgetModel::readValueFrom for path:" << path;
        beginResetModel();
        int i = 0;
        values_.clear();
        while (true) {
            auto value = readVariant(map, QString("%1%2%3")
                                              .arg(path)
                                              .arg(path.isEmpty() ? "" : "/")
                                              .arg(i));
            if (value.isNull()) {
                break;
            }
            values_ << value;
            i++;
        }
        endResetModel();
        // qCDebug(KCM_FCITX5) << "Exiting ListOptionWidgetModel::readValueFrom";
    }

    void writeValueTo(QVariantMap &map, const QString &path) {
        // qCDebug(KCM_FCITX5) << "Entering ListOptionWidgetModel::writeValueTo for path:" << path;
        int i = 0;
        for (auto &value : values_) {
            writeVariant(map, QString("%1/%2").arg(path).arg(i), value);
            i++;
        }
        if (!i) {
            map[path] = QVariantMap();
        }
        // qCDebug(KCM_FCITX5) << "Exiting ListOptionWidgetModel::writeValueTo";
    }

    void addItem(QVariant value) {
        // qCDebug(KCM_FCITX5) << "Entering ListOptionWidgetModel::addItem";
        beginInsertRows(QModelIndex(), values_.size(), values_.size());
        values_.append(value);
        endInsertRows();
        // qCDebug(KCM_FCITX5) << "Exiting ListOptionWidgetModel::addItem";
    }

    void editItem(const QModelIndex &index, QVariant value) {
        // qCDebug(KCM_FCITX5) << "Entering ListOptionWidgetModel::editItem for index" << index.row();
        if (!index.isValid() || index.row() >= values_.size()) {
            // qCWarning(KCM_FCITX5) << "Invalid index for editItem:" << index.row();
            return;
        }

        values_[index.row()] = value;
        Q_EMIT dataChanged(index, index);
        // qCDebug(KCM_FCITX5) << "Exiting ListOptionWidgetModel::editItem";
    }

    void removeItem(const QModelIndex &index) {
        // qCDebug(KCM_FCITX5) << "Entering ListOptionWidgetModel::removeItem for index" << index.row();
        if (!index.isValid() || index.row() >= values_.size()) {
            // qCWarning(listOptionWidget) << "Invalid index for removeItem:" << index.row();
            return;
        }
        beginRemoveRows(index.parent(), index.row(), index.row());
        values_.removeAt(index.row());
        endRemoveRows();
        // qCDebug(KCM_FCITX5) << "Exiting ListOptionWidgetModel::removeItem";
    }

    void moveUpItem(const QModelIndex &index) {
        // qCDebug(KCM_FCITX5) << "Entering ListOptionWidgetModel::moveUpItem for index" << index.row();
        if (!index.isValid() || index.row() >= values_.size() ||
            index.row() == 0) {
            // qCWarning(KCM_FCITX5) << "Invalid index for moveUpItem:" << index.row();
            return;
        }
        Q_EMIT layoutAboutToBeChanged();
        if (!beginMoveRows(index.parent(), index.row(), index.row(),
                           index.parent(), index.row() - 1)) {
            // qCWarning(KCM_FCITX5) << "beginMoveRows failed for moveUpItem";
            return;
        }
#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
        values_.swap(index.row() - 1, index.row());
#else
        values_.swapItemsAt(index.row() - 1, index.row());
#endif
        endMoveRows();
        // qCDebug(KCM_FCITX5) << "Exiting ListOptionWidgetModel::moveUpItem";
    }

    void moveDownItem(const QModelIndex &index) {
        // qCDebug(KCM_FCITX5) << "Entering ListOptionWidgetModel::moveDownItem for index" << index.row();
        if (!index.isValid() || index.row() >= values_.size() ||
            index.row() + 1 == values_.size()) {
            // qCWarning(KCM_FCITX5) << "Invalid index for moveDownItem:" << index.row();
            return;
        }
        if (!beginMoveRows(index.parent(), index.row(), index.row(),
                           index.parent(), index.row() + 2)) {
            // qCWarning(KCM_FCITX5) << "beginMoveRows failed for moveDownItem";
            return;
        }
#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
        values_.swap(index.row(), index.row() + 1);
#else
        values_.swapItemsAt(index.row(), index.row() + 1);
#endif
        endMoveRows();
        // qCDebug(KCM_FCITX5) << "Exiting ListOptionWidgetModel::moveDownItem";
    }

private:
    QList<QVariant> values_;
    ListOptionWidget *parent_;
};

ListOptionWidget::ListOptionWidget(const FcitxQtConfigOption &option,
                                   const QString &path, QWidget *parent)
    : OptionWidget(path, parent), model_(new ListOptionWidgetModel(this)),
      subOption_(option) {
    qCDebug(KCM_FCITX5) << "Entering ListOptionWidget constructor for path:" << path;
    setupUi(this);
    listView->setModel(model_);

    subOption_.setType(option.type().mid(5)); // Remove List|
    auto props = option.properties();
    if (props.contains("ListConstrain")) {
        auto itemConstrain = props.value("ListConstrain").toMap();
        props.remove("ListConstrain");
        for (auto iter = itemConstrain.begin(), end = itemConstrain.end();
             iter != end; ++iter) {
            props[iter.key()] = iter.value();
        }
    }
    subOption_.setProperties(props);
    subOption_.setDefaultValue(QDBusVariant());

    connect(listView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, [this]() { updateButton(); });

    connect(model_, &QAbstractListModel::rowsMoved, this,
            [this]() { updateButton(); });
    connect(addButton, &QAbstractButton::clicked, this, [this]() {
        QVariant result;
        qCDebug(KCM_FCITX5) << "Add button clicked";
        auto ok = OptionWidget::execOptionDialog(this, subOption_, result);
        if (ok) {
            model_->addItem(result);
        }
    });
    connect(editButton, &QAbstractButton::clicked, this, [this]() {
        QVariant result = model_->data(listView->currentIndex(), Qt::UserRole);
        qCDebug(KCM_FCITX5) << "Editing item at row:" << listView->currentIndex().row();
        auto ok = OptionWidget::execOptionDialog(this, subOption_, result);
        if (ok) {
            model_->editItem(listView->currentIndex(), result);
        }
    });
    connect(removeButton, &QAbstractButton::clicked, this,
            [this]() {
                qCDebug(KCM_FCITX5) << "Removing item at row:" << listView->currentIndex().row();
                model_->removeItem(listView->currentIndex());
            });
    connect(moveUpButton, &QAbstractButton::clicked, this,
            [this]() {
                qCDebug(KCM_FCITX5) << "Moving item up at row:" << listView->currentIndex().row();
                model_->moveUpItem(listView->currentIndex());
            });
    connect(moveDownButton, &QAbstractButton::clicked, this,
            [this]() {
                qCDebug(KCM_FCITX5) << "Moving item down at row:" << listView->currentIndex().row();
                model_->moveDownItem(listView->currentIndex());
            });

    auto variant = option.defaultValue().variant();
    if (variant.canConvert<QDBusArgument>()) {
        auto argument = qvariant_cast<QDBusArgument>(variant);
        argument >> defaultValue_;
    }

    updateButton();
    qCDebug(KCM_FCITX5) << "Exiting ListOptionWidget constructor";
}

void ListOptionWidget::updateButton() {
    qCDebug(KCM_FCITX5) << "Entering updateButton";
    editButton->setEnabled(listView->currentIndex().isValid());
    removeButton->setEnabled(listView->currentIndex().isValid());
    moveUpButton->setEnabled(listView->currentIndex().isValid() &&
                             listView->currentIndex().row() != 0);
    moveDownButton->setEnabled(listView->currentIndex().isValid() &&
                               listView->currentIndex().row() !=
                                   model_->rowCount() - 1);
    qCDebug(KCM_FCITX5) << "Exiting updateButton";
}

void ListOptionWidget::readValueFrom(const QVariantMap &map) {
    // qCDebug(KCM_FCITX5) << "Reading list values from configuration for path:" << path();
    model_->readValueFrom(map, path());
}

void ListOptionWidget::writeValueTo(QVariantMap &map) {
    // qCDebug(KCM_FCITX5) << "Writing list values to configuration for path:" << path();
    model_->writeValueTo(map, path());
}

void ListOptionWidget::restoreToDefault() {
    // qCDebug(KCM_FCITX5) << "Restoring list to default values";
    model_->readValueFrom(defaultValue_, "");
}

} // namespace kcm
} // namespace fcitx
