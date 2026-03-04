#pragma once

#include "gui/AppState.hpp"
#include "services/TaskService.hpp"

#include <QWidget>

class QListWidget;
class QLabel;
class QLineEdit;
class QPushButton;
class QShortcut;

class TasksWidget : public QWidget {
  Q_OBJECT

 public:
  TasksWidget(AppState& state, TaskService& task_service, QWidget* parent = nullptr);

 signals:
  void StatusMessage(const QString& message);

 public slots:
  void RefreshTasks();

 protected:
  void keyPressEvent(QKeyEvent* event) override;

 private slots:
  void OnSelectionChanged();
  void NewTask();
  void MarkDone();
  void DeleteTask();
  void OnFilterChanged(const QString& text);

 private:
  AppState& state_;
  TaskService& task_service_;

 QListWidget* list_ = nullptr;
 QLabel* details_ = nullptr;
 QLineEdit* filter_ = nullptr;
 QPushButton* new_button_ = nullptr;
 QPushButton* done_button_ = nullptr;
 QPushButton* delete_button_ = nullptr;
  std::vector<int> visible_indices_;

  void UpdateList();
  void UpdateDetails();
  void SetStatus(const QString& message);
};
