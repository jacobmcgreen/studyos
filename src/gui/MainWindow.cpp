#include "gui/MainWindow.hpp"

#include "gui/StatsWidget.hpp"
#include "gui/TasksWidget.hpp"
#include "gui/TimerWidget.hpp"
#include "gui/ScheduleWidget.hpp"

#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(TaskService& task_service, SessionService& session_service,
                       PreferenceService& preference_service, BusyBlockService& busy_block_service,
                       QWidget* parent)
    : QMainWindow(parent),
      task_service_(task_service),
      session_service_(session_service),
      preference_service_(preference_service),
      busy_block_service_(busy_block_service) {
  tabs_ = new QTabWidget(this);

  timer_ = new TimerWidget(state_, session_service_, tabs_);
  tasks_ = new TasksWidget(state_, task_service_, tabs_);
  stats_ = new StatsWidget(state_, session_service_, tabs_);
  schedule_ = new ScheduleWidget(task_service_, preference_service_, busy_block_service_, tabs_);

  tabs_->addTab(timer_, "Timer");
  tabs_->addTab(tasks_, "Tasks");
  tabs_->addTab(stats_, "Stats");
  tabs_->addTab(schedule_, "Schedule");

  setCentralWidget(tabs_);
  statusBar()->showMessage("Ready");

  connect(tabs_, &QTabWidget::currentChanged, this, &MainWindow::OnTabChanged);
  connect(tasks_, &TasksWidget::StatusMessage, this, &MainWindow::SetStatus);
  connect(timer_, &TimerWidget::StatusMessage, this, &MainWindow::SetStatus);
  connect(schedule_, &ScheduleWidget::StatusMessage, this, &MainWindow::SetStatus);

  statusBar()->showMessage(
      "Keys: Timer(s/p/x/r), Tasks(n/d, / filter, j/k), Schedule(generate/add), Tabs(ctrl+tab)");
}

void MainWindow::OnTabChanged(int index) {
  state_.tab_index = index;
  if (index == 1) {
    RefreshTasks();
  } else if (index == 2) {
    RefreshStats();
  }
}

void MainWindow::RefreshTasks() {
  tasks_->RefreshTasks();
}

void MainWindow::RefreshStats() {
  stats_->RefreshStats();
}

void MainWindow::SetStatus(const QString& message) {
  statusBar()->showMessage(message, 5000);
}
