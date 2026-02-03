#include "gui/ScheduleWidget.hpp"

#include "gui/dialogs/BusyBlockDialog.hpp"
#include "gui/dialogs/PreferenceDialog.hpp"
#include "gui/models/ScheduleTableModel.hpp"

#include <QDateEdit>
#include <QDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QTimeEdit>
#include <QVBoxLayout>

ScheduleWidget::ScheduleWidget(TaskService& task_service, PreferenceService& preference_service,
                               BusyBlockService& busy_block_service, QWidget* parent)
    : QWidget(parent),
      task_service_(task_service),
      preference_service_(preference_service),
      busy_block_service_(busy_block_service) {
  date_edit_ = new QDateEdit(QDate::currentDate(), this);
  date_edit_->setCalendarPopup(true);
  start_edit_ = new QTimeEdit(QTime(9, 0), this);
  end_edit_ = new QTimeEdit(QTime(19, 0), this);

  generate_button_ = new QPushButton("Generate Schedule", this);
  add_busy_button_ = new QPushButton("Add Busy Block", this);
  add_pref_button_ = new QPushButton("Add Preference", this);

  model_ = new ScheduleTableModel(this);
  table_ = new QTableView(this);
  table_->setModel(model_);
  table_->horizontalHeader()->setStretchLastSection(true);

  auto top_row = new QHBoxLayout();
  top_row->addWidget(new QLabel("Date", this));
  top_row->addWidget(date_edit_);
  top_row->addWidget(new QLabel("Start", this));
  top_row->addWidget(start_edit_);
  top_row->addWidget(new QLabel("End", this));
  top_row->addWidget(end_edit_);
  top_row->addWidget(generate_button_);
  top_row->addStretch(1);

  auto buttons_row = new QHBoxLayout();
  buttons_row->addWidget(add_busy_button_);
  buttons_row->addWidget(add_pref_button_);
  buttons_row->addStretch(1);

  auto layout = new QVBoxLayout();
  layout->addLayout(top_row);
  layout->addLayout(buttons_row);
  layout->addWidget(table_);
  setLayout(layout);

  connect(generate_button_, &QPushButton::clicked, this, &ScheduleWidget::GenerateSchedule);
  connect(add_busy_button_, &QPushButton::clicked, this, &ScheduleWidget::AddBusyBlock);
  connect(add_pref_button_, &QPushButton::clicked, this, &ScheduleWidget::AddPreference);
}

void ScheduleWidget::GenerateSchedule() {
  try {
    auto tasks = task_service_.listTasks();
    auto prefs = preference_service_.listPreferences();
    auto date = date_edit_->date().toString("yyyy-MM-dd").toStdString();
    auto busy = busy_block_service_.listBusyBlocksForDate(date);

    auto schedule = scheduler_.build_day_schedule(
        date,
        start_edit_->time().toString("HH:mm").toStdString(),
        end_edit_->time().toString("HH:mm").toStdString(),
        tasks, prefs, busy);

    model_->setItems(std::move(schedule));
    SetStatus("Schedule generated.");
  } catch (const std::exception& ex) {
    SetStatus(ex.what());
  }
}

void ScheduleWidget::AddBusyBlock() {
  BusyBlockDialog dialog(date_edit_->date(), this);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  auto block = dialog.block();
  if (block.title.empty()) {
    SetStatus("Busy block title required.");
    return;
  }
  try {
    busy_block_service_.addBusyBlock(block);
    SetStatus("Busy block added.");
  } catch (const std::exception& ex) {
    SetStatus(ex.what());
  }
}

void ScheduleWidget::AddPreference() {
  PreferenceDialog dialog(this);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  auto pref = dialog.preference();
  if (pref.tag.empty() && pref.energy.empty()) {
    SetStatus("Preference needs tag or energy.");
    return;
  }
  try {
    preference_service_.addPreference(pref);
    SetStatus("Preference added.");
  } catch (const std::exception& ex) {
    SetStatus(ex.what());
  }
}

void ScheduleWidget::SetStatus(const QString& message) {
  emit StatusMessage(message);
}
