#pragma once

#include "scheduler/Scheduler.hpp"

#include <QAbstractTableModel>

class ScheduleTableModel : public QAbstractTableModel {
  Q_OBJECT

 public:
  explicit ScheduleTableModel(QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

  void setItems(std::vector<ScheduleItem> items);

 private:
  std::vector<ScheduleItem> items_;
};
