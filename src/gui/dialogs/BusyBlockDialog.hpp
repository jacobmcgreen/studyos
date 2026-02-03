#pragma once

#include "models.hpp"

#include <QDialog>

class QDateEdit;
class QTimeEdit;
class QLineEdit;

class BusyBlockDialog : public QDialog {
  Q_OBJECT

 public:
  explicit BusyBlockDialog(const QDate& date, QWidget* parent = nullptr);
  BusyBlock block() const;

 private:
  QDateEdit* date_edit_ = nullptr;
  QTimeEdit* start_edit_ = nullptr;
  QTimeEdit* end_edit_ = nullptr;
  QLineEdit* title_edit_ = nullptr;
};
