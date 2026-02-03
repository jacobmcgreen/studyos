#include "gui/dialogs/BusyBlockDialog.hpp"

#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QTimeEdit>
#include <QVBoxLayout>

BusyBlockDialog::BusyBlockDialog(const QDate& date, QWidget* parent)
    : QDialog(parent) {
  setWindowTitle("Add Busy Block");

  date_edit_ = new QDateEdit(date, this);
  date_edit_->setCalendarPopup(true);
  start_edit_ = new QTimeEdit(QTime(9, 0), this);
  end_edit_ = new QTimeEdit(QTime(10, 0), this);
  title_edit_ = new QLineEdit(this);

  auto form = new QFormLayout();
  form->addRow("Date", date_edit_);
  form->addRow("Start", start_edit_);
  form->addRow("End", end_edit_);
  form->addRow("Title", title_edit_);

  auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  auto layout = new QVBoxLayout(this);
  layout->addLayout(form);
  layout->addWidget(buttons);
}

BusyBlock BusyBlockDialog::block() const {
  BusyBlock b;
  b.date = date_edit_->date().toString("yyyy-MM-dd").toStdString();
  b.start_time = start_edit_->time().toString("HH:mm").toStdString();
  b.end_time = end_edit_->time().toString("HH:mm").toStdString();
  b.title = title_edit_->text().toStdString();
  return b;
}
