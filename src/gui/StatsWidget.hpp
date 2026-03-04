#pragma once

#include "gui/AppState.hpp"
#include "services/SessionService.hpp"

#include <QWidget>

class QLabel;
class QTimer;

class StatsWidget : public QWidget {
  Q_OBJECT

 public:
  StatsWidget(AppState& state, SessionService& session_service, QWidget* parent = nullptr);

 public slots:
  void RefreshStats();

 private slots:
  void OnTick();

 private:
  AppState& state_;
  SessionService& session_service_;

  QLabel* today_label_ = nullptr;
  QLabel* week_label_ = nullptr;
  QLabel* today_tags_label_ = nullptr;
  QLabel* tags_label_ = nullptr;
  QTimer* tick_timer_ = nullptr;

  void UpdateUi();
  void RecomputeStats();
  static QString FormatDuration(long long seconds);
  static QString BarsForSeconds(long long seconds, long long max_seconds);
};
