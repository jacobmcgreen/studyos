#include "gui/TimerWidget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QShortcut>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>

#include <ctime>

namespace {

QString ModeToString(AppState::TimerState::Mode mode) {
  switch (mode) {
    case AppState::TimerState::Mode::Idle:
      return "IDLE";
    case AppState::TimerState::Mode::Focus:
      return "FOCUS";
    case AppState::TimerState::Mode::Break:
      return "BREAK";
  }
  return "IDLE";
}

QString FormatDuration(int seconds) {
  int minutes = seconds / 60;
  int secs = seconds % 60;
  return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
}

}  // namespace

TimerWidget::TimerWidget(AppState& state, SessionService& session_service, QWidget* parent)
    : QWidget(parent), state_(state), session_service_(session_service) {
  session_label_ = new QLabel("Session #0", this);
  session_label_->setStyleSheet("color: #6c7086; font-size: 12px;");
  session_label_->setAlignment(Qt::AlignCenter);

  mode_label_ = new QLabel(this);
  mode_label_->setStyleSheet("font-weight: bold; font-size: 18px; color: #cba6f7;");
  mode_label_->setAlignment(Qt::AlignCenter);

  time_label_ = new QLabel(this);
  time_label_->setStyleSheet("font-size: 52px; font-weight: bold; font-family: monospace;");
  time_label_->setAlignment(Qt::AlignCenter);

  progress_bar_ = new QProgressBar(this);
  progress_bar_->setRange(0, 1000);
  progress_bar_->setValue(1000);
  progress_bar_->setTextVisible(false);
  progress_bar_->setFixedHeight(6);

  tag_edit_ = new QLineEdit(this);
  tag_edit_->setPlaceholderText("focus tag");

  focus_minutes_ = new QSpinBox(this);
  focus_minutes_->setRange(1, 120);
  focus_minutes_->setValue(25);
  focus_minutes_->setSuffix(" min focus");

  break_minutes_ = new QSpinBox(this);
  break_minutes_->setRange(1, 60);
  break_minutes_->setValue(5);
  break_minutes_->setSuffix(" min break");

  start_button_ = new QPushButton("Start", this);
  pause_button_ = new QPushButton("Pause", this);
  stop_button_ = new QPushButton("Stop", this);
  reset_button_ = new QPushButton("Reset", this);

  auto duration_row = new QHBoxLayout();
  duration_row->addWidget(focus_minutes_);
  duration_row->addWidget(break_minutes_);

  auto buttons = new QHBoxLayout();
  buttons->addWidget(start_button_);
  buttons->addWidget(pause_button_);
  buttons->addWidget(stop_button_);
  buttons->addWidget(reset_button_);

  auto layout = new QVBoxLayout();
  layout->setSpacing(8);
  layout->addStretch(1);
  layout->addWidget(session_label_);
  layout->addWidget(mode_label_);
  layout->addWidget(time_label_);
  layout->addWidget(progress_bar_);
  layout->addSpacing(12);
  layout->addWidget(tag_edit_);
  layout->addLayout(duration_row);
  layout->addLayout(buttons);
  layout->addStretch(2);
  setLayout(layout);

  tick_timer_ = new QTimer(this);
  tick_timer_->setInterval(1000);
  tick_timer_->start();

  connect(tick_timer_, &QTimer::timeout, this, &TimerWidget::OnTick);
  connect(start_button_, &QPushButton::clicked, this, &TimerWidget::StartFocus);
  connect(pause_button_, &QPushButton::clicked, this, &TimerWidget::PauseResume);
  connect(stop_button_, &QPushButton::clicked, this, &TimerWidget::StopSession);
  connect(reset_button_, &QPushButton::clicked, this, &TimerWidget::ResetTimer);

  auto start_shortcut = new QShortcut(QKeySequence("s"), this);
  auto pause_shortcut = new QShortcut(QKeySequence("p"), this);
  auto stop_shortcut = new QShortcut(QKeySequence("x"), this);
  auto reset_shortcut = new QShortcut(QKeySequence("r"), this);
  connect(start_shortcut, &QShortcut::activated, this, &TimerWidget::StartFocus);
  connect(pause_shortcut, &QShortcut::activated, this, &TimerWidget::PauseResume);
  connect(stop_shortcut, &QShortcut::activated, this, &TimerWidget::StopSession);
  connect(reset_shortcut, &QShortcut::activated, this, &TimerWidget::ResetTimer);

  UpdateUi();
}

void TimerWidget::OnTick() {
  if (!state_.timer.running || state_.timer.paused) {
    return;
  }

  if (state_.timer.remaining_seconds > 0) {
    state_.timer.remaining_seconds -= 1;
    UpdateUi();
    return;
  }

  if (state_.timer.mode == AppState::TimerState::Mode::Focus) {
    SwitchToBreak();
  } else if (state_.timer.mode == AppState::TimerState::Mode::Break) {
    SwitchToFocusIdle();
  }
}

void TimerWidget::StartFocus() {
  if (state_.timer.running && state_.timer.mode == AppState::TimerState::Mode::Focus) {
    SetStatus("Focus already running.");
    return;
  }

  if (session_service_.getActiveSession().has_value()) {
    SetStatus("A session is already active. Stop it before starting a new one.");
    return;
  }

  QString tag = tag_edit_->text().trimmed();
  if (tag.isEmpty()) {
    if (state_.timer.last_tag.empty()) {
      SetStatus("Enter a tag to start.");
      return;
    }
    tag = QString::fromStdString(state_.timer.last_tag);
  }

  try {
    session_service_.startSession(tag.toStdString(), std::time(nullptr));
    state_.timer.session_active = true;
  } catch (const std::exception& ex) {
    SetStatus(ex.what());
    return;
  }

  state_.timer.last_tag = tag.toStdString();
  state_.timer.mode = AppState::TimerState::Mode::Focus;
  state_.timer.remaining_seconds = focus_minutes_->value() * 60;
  state_.timer.total_seconds = state_.timer.remaining_seconds;
  state_.timer.running = true;
  state_.timer.paused = false;
  UpdateUi();
  SetStatus("Focus started.");
}

void TimerWidget::PauseResume() {
  if (!state_.timer.running) {
    SetStatus("No active timer.");
    return;
  }
  state_.timer.paused = !state_.timer.paused;
  pause_button_->setText(state_.timer.paused ? "Resume" : "Pause");
  SetStatus(state_.timer.paused ? "Paused." : "Resumed.");
}

void TimerWidget::StopSession() {
  if (session_service_.getActiveSession().has_value()) {
    try {
      session_service_.stopActiveSession(std::time(nullptr));
    } catch (const std::exception& ex) {
      SetStatus(ex.what());
      return;
    }
    state_.timer.session_active = false;
  }

  state_.timer.running = false;
  state_.timer.paused = false;
  state_.timer.mode = AppState::TimerState::Mode::Idle;
  state_.timer.remaining_seconds = focus_minutes_->value() * 60;
  UpdateUi();
  SetStatus("Session stopped.");
}

void TimerWidget::ResetTimer() {
  state_.timer.remaining_seconds = focus_minutes_->value() * 60;
  state_.timer.mode = AppState::TimerState::Mode::Idle;
  state_.timer.running = false;
  state_.timer.paused = false;
  UpdateUi();
  SetStatus("Timer reset.");
}

void TimerWidget::UpdateUi() {
  const auto mode = state_.timer.mode;
  using Mode = AppState::TimerState::Mode;

  mode_label_->setText(ModeToString(mode));
  time_label_->setText(FormatDuration(state_.timer.remaining_seconds));
  pause_button_->setText(state_.timer.paused ? "Resume" : "Pause");

  // Session counter label.
  if (state_.timer.session_count > 0) {
    session_label_->setText(QString("Session #%1").arg(state_.timer.session_count));
  }

  // Progress bar: fraction of time remaining.
  int total = state_.timer.total_seconds > 0 ? state_.timer.total_seconds : 1;
  int val = static_cast<int>((static_cast<double>(state_.timer.remaining_seconds) / total) * 1000);
  progress_bar_->setValue(val);

  // Color progress bar and mode label by mode.
  if (mode == Mode::Focus) {
    mode_label_->setStyleSheet("font-weight: bold; font-size: 18px; color: #cba6f7;");
    progress_bar_->setStyleSheet("QProgressBar::chunk { background: #cba6f7; border-radius: 3px; }");
  } else if (mode == Mode::Break) {
    mode_label_->setStyleSheet("font-weight: bold; font-size: 18px; color: #a6e3a1;");
    progress_bar_->setStyleSheet("QProgressBar::chunk { background: #a6e3a1; border-radius: 3px; }");
  } else {
    mode_label_->setStyleSheet("font-weight: bold; font-size: 18px; color: #6c7086;");
    progress_bar_->setStyleSheet("QProgressBar::chunk { background: #45475a; border-radius: 3px; }");
  }

  // Window title.
  QString title;
  if (mode == Mode::Focus) {
    title = QString("%1 FOCUS [%2] — StudyOS")
                .arg(FormatDuration(state_.timer.remaining_seconds))
                .arg(QString::fromStdString(state_.timer.last_tag));
  } else if (mode == Mode::Break) {
    title = QString("%1 BREAK — StudyOS")
                .arg(FormatDuration(state_.timer.remaining_seconds));
  } else {
    title = "StudyOS";
  }
  emit TitleChanged(title);
}

void TimerWidget::SwitchToBreak() {
  if (state_.timer.session_active) {
    try {
      session_service_.stopActiveSession(std::time(nullptr));
    } catch (const std::exception& ex) {
      SetStatus(ex.what());
    }
    state_.timer.session_active = false;
  }
  state_.timer.mode = AppState::TimerState::Mode::Break;
  state_.timer.remaining_seconds = break_minutes_->value() * 60;
  state_.timer.total_seconds = state_.timer.remaining_seconds;
  state_.timer.session_count += 1;
  state_.timer.running = true;
  state_.timer.paused = false;
  UpdateUi();
  SetStatus("Break started.");
}

void TimerWidget::SwitchToFocusIdle() {
  state_.timer.mode = AppState::TimerState::Mode::Idle;
  state_.timer.remaining_seconds = focus_minutes_->value() * 60;
  state_.timer.running = false;
  state_.timer.paused = false;
  UpdateUi();
  SetStatus("Break complete. Ready for focus.");
}

void TimerWidget::SetStatus(const QString& message) {
  emit StatusMessage(message);
}
