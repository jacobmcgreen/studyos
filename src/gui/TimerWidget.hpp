#pragma once

#include "gui/AppState.hpp"
#include "services/SessionService.hpp"

#include <QWidget>

class QLabel;
class QPushButton;
class QLineEdit;
class QTimer;

class TimerWidget : public QWidget {
  Q_OBJECT

 public:
  TimerWidget(AppState& state, SessionService& session_service, QWidget* parent = nullptr);

 signals:
  void StatusMessage(const QString& message);

 private slots:
  void OnTick();
  void StartFocus();
  void PauseResume();
  void StopSession();
  void ResetTimer();

 private:
  AppState& state_;
  SessionService& session_service_;

  QLabel* mode_label_ = nullptr;
  QLabel* time_label_ = nullptr;
  QLineEdit* tag_edit_ = nullptr;
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
