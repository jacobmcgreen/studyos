#include "gui/dialogs/PreferenceDialog.hpp"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QTimeEdit>
#include <QVBoxLayout>

PreferenceDialog::PreferenceDialog(QWidget* parent) : QDialog(parent) {
  setWindowTitle("Add Preference");

  name_edit_ = new QLineEdit(this);
  tag_edit_ = new QLineEdit(this);
  energy_combo_ = new QComboBox(this);
  energy_combo_->addItem("");
  energy_combo_->addItem("low");
  energy_combo_->addItem("med");
  energy_combo_->addItem("high");
  start_edit_ = new QTimeEdit(QTime(9, 0), this);
  end_edit_ = new QTimeEdit(QTime(10, 0), this);
  weight_spin_ = new QSpinBox(this);
  weight_spin_->setRange(1, 10);
  weight_spin_->setValue(5);

  auto form = new QFormLayout();
  form->addRow("Name", name_edit_);
  form->addRow("Tag", tag_edit_);
  form->addRow("Energy", energy_combo_);
  form->addRow("Start", start_edit_);
  form->addRow("End", end_edit_);
  form->addRow("Weight", weight_spin_);

  auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  auto layout = new QVBoxLayout(this);
  layout->addLayout(form);
  layout->addWidget(buttons);
}

Preference PreferenceDialog::preference() const {
  Preference p;
  p.name = name_edit_->text().toStdString();
  p.tag = tag_edit_->text().toStdString();
  p.energy = energy_combo_->currentText().toStdString();
  p.start_time = start_edit_->time().toString("HH:mm").toStdString();
  p.end_time = end_edit_->time().toString("HH:mm").toStdString();
  p.weight = weight_spin_->value();
  return p;
}
