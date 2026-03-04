#pragma once

#include "gui/AppState.hpp"
#include "services/SessionService.hpp"

#include <QWidget>

class QLabel;
class QPushButton;
class QLineEdit;
class QProgressBar;
class QSpinBox;
class QTimer;

class TimerWidget : public QWidget {
  Q_OBJECT

 public:
  TimerWidget(AppState& state, SessionService& session_service, QWidget* parent = nullptr);

 signals:
  void StatusMessage(const QString& message);
  void TitleChanged(const QString& title);

 private slots:
  void OnTick();
  void StartFocus();
  void PauseResume();
  void StopSession();
  void ResetTimer();

 private:
  AppState& state_;
  SessionService& session_service_;

  QLabel* session_label_ = nullptr;
  QLabel* mode_label_ = nullptr;
  QLabel* time_label_ = nullptr;
  QProgressBar* progress_bar_ = nullptr;
  QLineEdit* tag_edit_ = nullptr;
  QSpinBox* focus_minutes_ = nullptr;
  QSpinBox* break_minutes_ = nullptr;
  QPushButton* start_button_ = nullptr;
  QPushButton* pause_button_ = nullptr;
  QPushButton* stop_button_ = nullptr;
  QPushButton* reset_button_ = nullptr;
  QTimer* tick_timer_ = nullptr;

  void UpdateUi();
  void SwitchToBreak();
  void SwitchToFocusIdle();
  void SetStatus(const QString& message);
};
