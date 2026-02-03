#pragma once

#include "scheduler/Scheduler.hpp"
#include "services/BusyBlockService.hpp"
#include "services/PreferenceService.hpp"
#include "services/TaskService.hpp"

#include <QWidget>

class QDateEdit;
class QTimeEdit;
class QPushButton;
class QTableView;
class ScheduleTableModel;

class ScheduleWidget : public QWidget {
  Q_OBJECT

 public:
  ScheduleWidget(TaskService& task_service, PreferenceService& preference_service,
                 BusyBlockService& busy_block_service, QWidget* parent = nullptr);

 signals:
  void StatusMessage(const QString& message);

 private slots:
  void GenerateSchedule();
  void AddBusyBlock();
  void AddPreference();

 private:
  TaskService& task_service_;
  PreferenceService& preference_service_;
  BusyBlockService& busy_block_service_;
  Scheduler scheduler_;

  QDateEdit* date_edit_ = nullptr;
  QTimeEdit* start_edit_ = nullptr;
  QTimeEdit* end_edit_ = nullptr;
  QPushButton* generate_button_ = nullptr;
  QPushButton* add_busy_button_ = nullptr;
  QPushButton* add_pref_button_ = nullptr;
  QTableView* table_ = nullptr;
  ScheduleTableModel* model_ = nullptr;

  void SetStatus(const QString& message);
};
