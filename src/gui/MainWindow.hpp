#pragma once

#include "gui/AppState.hpp"
#include "services/BusyBlockService.hpp"
#include "services/PreferenceService.hpp"
#include "services/SessionService.hpp"
#include "services/TaskService.hpp"

#include <QMainWindow>

class TimerWidget;
class TasksWidget;
class StatsWidget;
class ScheduleWidget;
class QTabWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(TaskService& task_service, SessionService& session_service,
             PreferenceService& preference_service, BusyBlockService& busy_block_service,
             QWidget* parent = nullptr);

 private slots:
  void OnTabChanged(int index);
  void RefreshTasks();
  void RefreshStats();

 private:
  TaskService& task_service_;
  SessionService& session_service_;
  PreferenceService& preference_service_;
  BusyBlockService& busy_block_service_;
  AppState state_;

  QTabWidget* tabs_ = nullptr;
  TimerWidget* timer_ = nullptr;
  TasksWidget* tasks_ = nullptr;
  StatsWidget* stats_ = nullptr;
  ScheduleWidget* schedule_ = nullptr;

  void SetStatus(const QString& message);
};
