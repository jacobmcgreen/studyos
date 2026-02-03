#include "gui/TasksWidget.hpp"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QShortcut>
#include <QVBoxLayout>

#include <algorithm>

namespace {

QString TaskStatusText(const Task& task) {
  return task.completed ? "Completed" : "Open";
}

}  // namespace

TasksWidget::TasksWidget(AppState& state, TaskService& task_service, QWidget* parent)
    : QWidget(parent), state_(state), task_service_(task_service) {
  filter_ = new QLineEdit(this);
  filter_->setPlaceholderText("/ filter tasks");

  list_ = new QListWidget(this);
  details_ = new QLabel("Select a task", this);
  details_->setWordWrap(true);

  new_button_ = new QPushButton("New", this);
  done_button_ = new QPushButton("Done", this);

  auto list_layout = new QVBoxLayout();
  list_layout->addWidget(filter_);
  list_layout->addWidget(list_);

  auto right_layout = new QVBoxLayout();
  right_layout->addWidget(details_);
  right_layout->addStretch(1);
  right_layout->addWidget(new_button_);
  right_layout->addWidget(done_button_);

  auto layout = new QHBoxLayout();
  layout->addLayout(list_layout, 2);
  layout->addLayout(right_layout, 3);
  setLayout(layout);

  auto search_shortcut = new QShortcut(QKeySequence("/"), this);
  connect(search_shortcut, &QShortcut::activated, filter_, qOverload<>(&QWidget::setFocus));
  auto new_shortcut = new QShortcut(QKeySequence("n"), this);
  auto done_shortcut = new QShortcut(QKeySequence("d"), this);
  connect(new_shortcut, &QShortcut::activated, this, &TasksWidget::NewTask);
  connect(done_shortcut, &QShortcut::activated, this, &TasksWidget::MarkDone);

  connect(list_, &QListWidget::currentRowChanged, this, &TasksWidget::OnSelectionChanged);
  connect(new_button_, &QPushButton::clicked, this, &TasksWidget::NewTask);
  connect(done_button_, &QPushButton::clicked, this, &TasksWidget::MarkDone);
  connect(filter_, &QLineEdit::textChanged, this, &TasksWidget::OnFilterChanged);

  RefreshTasks();
}

void TasksWidget::RefreshTasks() {
  state_.tasks.tasks = task_service_.listTasks();
  UpdateList();
  UpdateDetails();
}

void TasksWidget::OnSelectionChanged() {
  state_.tasks.selected_index = list_->currentRow();
  UpdateDetails();
}

void TasksWidget::NewTask() {
  QDialog dialog(this);
  dialog.setWindowTitle("New Task");

  auto title = new QLineEdit(&dialog);
  auto tag = new QLineEdit(&dialog);
  auto due = new QLineEdit(&dialog);
  due->setPlaceholderText("YYYY-MM-DD");
  auto estimate = new QSpinBox(&dialog);
  estimate->setRange(30, 8 * 60);
  estimate->setSingleStep(30);
  estimate->setValue(60);
  auto priority = new QSpinBox(&dialog);
  priority->setRange(1, 5);
  priority->setValue(3);
  auto min_chunk = new QComboBox(&dialog);
  min_chunk->addItems({"30", "60"});
  auto splittable = new QCheckBox("Allow split", &dialog);
  splittable->setChecked(true);
  auto energy = new QComboBox(&dialog);
  energy->addItems({"low", "med", "high"});

  auto form = new QFormLayout();
  form->addRow("Title", title);
  form->addRow("Tag", tag);
  form->addRow("Due", due);
  form->addRow("Estimate (min)", estimate);
  form->addRow("Priority", priority);
  form->addRow("Min chunk", min_chunk);
  form->addRow("Splittable", splittable);
  form->addRow("Energy", energy);

  auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

  auto layout = new QVBoxLayout(&dialog);
  layout->addLayout(form);
  layout->addWidget(buttons);

  if (dialog.exec() == QDialog::Accepted) {
    if (title->text().trimmed().isEmpty()) {
      SetStatus("Title required.");
      return;
    }
    try {
      task_service_.addTask(title->text().toStdString(), tag->text().toStdString(),
                            due->text().toStdString(), estimate->value(), priority->value(),
                            min_chunk->currentText().toInt(), splittable->isChecked(),
                            energy->currentText().toStdString());
      SetStatus("Task added.");
      RefreshTasks();
    } catch (const std::exception& ex) {
      SetStatus(ex.what());
    }
  }
}

void TasksWidget::MarkDone() {
  if (state_.tasks.selected_index < 0 || state_.tasks.selected_index >= list_->count()) {
    SetStatus("Select a task.");
    return;
  }

  int task_index = visible_indices_[static_cast<size_t>(state_.tasks.selected_index)];
  const auto& task = state_.tasks.tasks[static_cast<size_t>(task_index)];
  if (task.completed) {
    SetStatus("Already completed.");
    return;
  }

  try {
    task_service_.markDone(task.id);
    SetStatus("Task marked done.");
    RefreshTasks();
  } catch (const std::exception& ex) {
    SetStatus(ex.what());
  }
}

void TasksWidget::OnFilterChanged(const QString& text) {
  state_.tasks.filter = text.toStdString();
  UpdateList();
  UpdateDetails();
}

void TasksWidget::UpdateList() {
  list_->clear();
  visible_indices_.clear();

  const QString filter = QString::fromStdString(state_.tasks.filter).toLower();
  for (size_t i = 0; i < state_.tasks.tasks.size(); ++i) {
    const auto& task = state_.tasks.tasks[i];
    QString title = QString::fromStdString(task.title);
    if (!filter.isEmpty() && !title.toLower().contains(filter)) {
      continue;
    }

    QString label = QString("[%1] %2%3")
                        .arg(task.id)
                        .arg(task.completed ? "[x] " : "[ ] ")
                        .arg(title);
    if (!task.tag.empty()) {
      label += QString(" (%1)").arg(QString::fromStdString(task.tag));
    }
    list_->addItem(label);
    visible_indices_.push_back(static_cast<int>(i));
  }

  if (list_->count() > 0) {
    list_->setCurrentRow(std::max(0, std::min(state_.tasks.selected_index, list_->count() - 1)));
  }
}

void TasksWidget::UpdateDetails() {
  int row = list_->currentRow();
  if (row < 0 || row >= static_cast<int>(visible_indices_.size())) {
    details_->setText("Select a task");
    return;
  }

  const auto& task = state_.tasks.tasks[static_cast<size_t>(visible_indices_[row])];
  QString text = QString("Title: %1\nTag: %2\nDue: %3\nStatus: %4\nEstimate: %5m\nPriority: %6\nMin chunk: %7\nSplittable: %8\nEnergy: %9")
                     .arg(QString::fromStdString(task.title))
                     .arg(task.tag.empty() ? "-" : QString::fromStdString(task.tag))
                     .arg(task.due_date.empty() ? "-" : QString::fromStdString(task.due_date))
                     .arg(TaskStatusText(task))
                     .arg(task.estimate_minutes)
                     .arg(task.priority)
                     .arg(task.min_chunk)
                     .arg(task.splittable ? "yes" : "no")
                     .arg(task.energy.empty() ? "-" : QString::fromStdString(task.energy));
  details_->setText(text);
}

void TasksWidget::SetStatus(const QString& message) {
  emit StatusMessage(message);
}

void TasksWidget::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_J) {
    list_->setCurrentRow(std::min(list_->currentRow() + 1, list_->count() - 1));
    return;
  }
  if (event->key() == Qt::Key_K) {
    list_->setCurrentRow(std::max(list_->currentRow() - 1, 0));
    return;
  }
  QWidget::keyPressEvent(event);
}
