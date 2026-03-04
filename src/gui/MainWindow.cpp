#include "gui/MainWindow.hpp"

#include "gui/StatsWidget.hpp"
#include "gui/TasksWidget.hpp"
#include "gui/TimerWidget.hpp"
#include "gui/ScheduleWidget.hpp"

#include <QShortcut>
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
  connect(timer_, &TimerWidget::TitleChanged, this, &MainWindow::setWindowTitle);
  connect(schedule_, &ScheduleWidget::StatusMessage, this, &MainWindow::SetStatus);

  auto tab1 = new QShortcut(QKeySequence("Ctrl+1"), this);
  auto tab2 = new QShortcut(QKeySequence("Ctrl+2"), this);
  auto tab3 = new QShortcut(QKeySequence("Ctrl+3"), this);
  auto tab4 = new QShortcut(QKeySequence("Ctrl+4"), this);
  connect(tab1, &QShortcut::activated, this, [this]() { tabs_->setCurrentIndex(0); });
  connect(tab2, &QShortcut::activated, this, [this]() { tabs_->setCurrentIndex(1); });
  connect(tab3, &QShortcut::activated, this, [this]() { tabs_->setCurrentIndex(2); });
  connect(tab4, &QShortcut::activated, this, [this]() { tabs_->setCurrentIndex(3); });

  statusBar()->showMessage(
      "Keys: Timer(s/p/x/r), Tasks(n/d/Del, / filter, j/k), Tabs(Ctrl+1-4)");
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
