#include "gui/models/ScheduleTableModel.hpp"

ScheduleTableModel::ScheduleTableModel(QObject* parent) : QAbstractTableModel(parent) {}

int ScheduleTableModel::rowCount(const QModelIndex& /*parent*/) const {
  return static_cast<int>(items_.size());
}

int ScheduleTableModel::columnCount(const QModelIndex& /*parent*/) const {
  return 4;
}

QVariant ScheduleTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || role != Qt::DisplayRole) {
    return {};
  }

  const auto& item = items_[static_cast<size_t>(index.row())];
  switch (index.column()) {
    case 0:
      return QString::fromStdString(item.start + " - " + item.end);
    case 1:
      return QString::fromStdString(item.title);
    case 2:
      return QString::fromStdString(item.tag);
    case 3:
      return QString::fromStdString(item.reason);
    default:
      return {};
  }
}

QVariant ScheduleTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
    return {};
  }
  switch (section) {
    case 0:
      return "Time";
    case 1:
      return "Task";
    case 2:
      return "Tag";
    case 3:
      return "Reason";
    default:
      return {};
  }
}

void ScheduleTableModel::setItems(std::vector<ScheduleItem> items) {
  beginResetModel();
  items_ = std::move(items);
  endResetModel();
}
