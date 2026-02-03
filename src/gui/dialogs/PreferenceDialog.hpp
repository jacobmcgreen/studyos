#pragma once

#include "models.hpp"

#include <QDialog>

class QLineEdit;
class QComboBox;
class QTimeEdit;
class QSpinBox;

class PreferenceDialog : public QDialog {
  Q_OBJECT

 public:
  explicit PreferenceDialog(QWidget* parent = nullptr);
  Preference preference() const;

 private:
  QLineEdit* name_edit_ = nullptr;
  QLineEdit* tag_edit_ = nullptr;
  QComboBox* energy_combo_ = nullptr;
  QTimeEdit* start_edit_ = nullptr;
  QTimeEdit* end_edit_ = nullptr;
  QSpinBox* weight_spin_ = nullptr;
};
