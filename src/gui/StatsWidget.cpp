#include "gui/StatsWidget.hpp"

#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <ctime>

namespace {

std::time_t StartOfDay(std::time_t now) {
  std::tm tm = *std::localtime(&now);
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  return std::mktime(&tm);
}

std::time_t StartOfWeek(std::time_t now) {
  std::tm tm = *std::localtime(&now);
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  int days_since_monday = (tm.tm_wday + 6) % 7;
  tm.tm_mday -= days_since_monday;
  return std::mktime(&tm);
}

}  // namespace

StatsWidget::StatsWidget(AppState& state, SessionService& session_service, QWidget* parent)
    : QWidget(parent), state_(state), session_service_(session_service) {
  today_label_ = new QLabel(this);
  week_label_ = new QLabel(this);
  today_tags_label_ = new QLabel(this);
  today_tags_label_->setWordWrap(true);
  tags_label_ = new QLabel(this);
  tags_label_->setWordWrap(true);

  auto layout = new QVBoxLayout();
  layout->addWidget(today_label_);
  layout->addWidget(new QLabel("Top tags (today)", this));
  layout->addWidget(today_tags_label_);
  layout->addWidget(week_label_);
  layout->addWidget(new QLabel("Top tags (week)", this));
  layout->addWidget(tags_label_);
  layout->addStretch(1);
  setLayout(layout);

  tick_timer_ = new QTimer(this);
  tick_timer_->setInterval(60000);
  tick_timer_->start();
  connect(tick_timer_, &QTimer::timeout, this, &StatsWidget::OnTick);

  RefreshStats();
}

void StatsWidget::RefreshStats() {
  RecomputeStats();
  UpdateUi();
}

void StatsWidget::OnTick() {
  RefreshStats();
}

void StatsWidget::RecomputeStats() {
  auto now = std::time(nullptr);
  auto day_start = StartOfDay(now);
  auto week_start = StartOfWeek(now);

  state_.stats.today_seconds = session_service_.totalSecondsForRange(day_start, day_start + 24 * 60 * 60);
  state_.stats.week_seconds = session_service_.totalSecondsForRange(week_start, week_start + 7 * 24 * 60 * 60);
  state_.stats.today_tags = session_service_.topTagsForRange(day_start, day_start + 24 * 60 * 60, 5);
  state_.stats.top_tags = session_service_.topTagsForRange(week_start, week_start + 7 * 24 * 60 * 60, 5);
}

void StatsWidget::UpdateUi() {
  today_label_->setText(QString("Today: %1").arg(FormatDuration(state_.stats.today_seconds)));
  week_label_->setText(QString("Week: %1").arg(FormatDuration(state_.stats.week_seconds)));

  auto make_tag_lines = [this](const std::vector<std::pair<std::string, long long>>& tags) -> QString {
    if (tags.empty()) return "No sessions yet.";
    long long max_seconds = 1;
    for (const auto& entry : tags) {
      max_seconds = std::max(max_seconds, entry.second);
    }
    QStringList lines;
    for (const auto& entry : tags) {
      lines << QString("%1 | %2 %3")
                 .arg(QString::fromStdString(entry.first))
                 .arg(FormatDuration(entry.second))
                 .arg(BarsForSeconds(entry.second, max_seconds));
    }
    return lines.join("\n");
  };

  today_tags_label_->setText(make_tag_lines(state_.stats.today_tags));
  tags_label_->setText(make_tag_lines(state_.stats.top_tags));
}

QString StatsWidget::FormatDuration(long long seconds) {
  long long minutes = seconds / 60;
  long long hrs = minutes / 60;
  long long mins = minutes % 60;
  return QString("%1h %2m").arg(hrs).arg(mins);
}

QString StatsWidget::BarsForSeconds(long long seconds, long long max_seconds) {
  if (max_seconds <= 0) {
    return "";
  }
  int width = 10;
  int count = static_cast<int>((static_cast<double>(seconds) / max_seconds) * width + 0.5);
  return QString(count, QChar('#'));
}
