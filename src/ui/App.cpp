#include "ui/App.hpp"

#include "ui/StatsView.hpp"
#include "ui/TasksView.hpp"
#include "ui/TimerView.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <atomic>
#include <ctime>
#include <thread>

namespace {

std::time_t start_of_day(std::time_t now) {
  std::tm tm = *std::localtime(&now);
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  return std::mktime(&tm);
}

std::time_t start_of_week(std::time_t now) {
  std::tm tm = *std::localtime(&now);
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  int days_since_monday = (tm.tm_wday + 6) % 7;
  tm.tm_mday -= days_since_monday;
  return std::mktime(&tm);
}

}  // namespace

App::App(TaskService& task_service, SessionService& session_service)
    : task_service_(task_service), session_service_(session_service) {
  RefreshTasks();
  RefreshStats();
}

void App::Run() {
  using namespace ftxui;

  auto screen = ScreenInteractive::Fullscreen();

  auto timer_view = MakeTimerView(state_, session_service_,
                                  [&](const std::string& msg) { SetStatus(msg); });
  auto tasks_view = MakeTasksView(state_, task_service_,
                                  [&] { RefreshTasks(); },
                                  [&](const std::string& msg) { SetStatus(msg); });
  auto stats_view = MakeStatsView(state_, session_service_, [&] { RefreshStats(); });

  auto tabs = Container::Tab({timer_view, tasks_view, stats_view}, &state_.tab_index);

  auto renderer = Renderer(tabs, [&] {
    auto tab_bar = hbox({
        text("Timer") | (state_.tab_index == 0 ? bold : dim),
        text("  Tasks") | (state_.tab_index == 1 ? bold : dim),
        text("  Stats") | (state_.tab_index == 2 ? bold : dim),
    });

    auto footer = hbox({
        text("Keys: Tab switch | s start | p pause | x stop | r reset | n new task | d done | / filter | q quit"),
    });

    if (!state_.status_message.empty()) {
      footer = hbox({text(state_.status_message)});
    }

    return vbox({
        tab_bar | border,
        tabs->Render() | flex,
        footer | border,
    });
  });

  auto root = CatchEvent(renderer, [&](Event event) {
    if (event == Event::Character('q')) {
      screen.Exit();
      return true;
    }
    if (event == Event::Tab) {
      state_.tab_index = (state_.tab_index + 1) % 3;
      if (state_.tab_index == 2) {
        RefreshStats();
      }
      return true;
    }
    if (event == Event::Custom) {
      Tick();
      return true;
    }
    return false;
  });

  std::atomic<bool> running = true;
  std::thread ticker([&] {
    while (running.load()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      screen.PostEvent(Event::Custom);
    }
  });

  screen.Loop(root);
  running.store(false);
  if (ticker.joinable()) {
    ticker.join();
  }
}

void App::RefreshTasks() {
  state_.tasks.tasks = task_service_.listTasks();
  if (state_.tasks.selected_index < 0) {
    state_.tasks.selected_index = 0;
  }
}

void App::RefreshStats() {
  auto now = std::time(nullptr);
  auto start_day = start_of_day(now);
  auto start_week = start_of_week(now);

  state_.stats.today_seconds = session_service_.totalSecondsForRange(start_day, start_day + 24 * 60 * 60);
  state_.stats.week_seconds = session_service_.totalSecondsForRange(start_week, start_week + 7 * 24 * 60 * 60);
  state_.stats.top_tags = session_service_.topTagsForRange(start_week, start_week + 7 * 24 * 60 * 60, 5);
  state_.stats.last_update = std::chrono::steady_clock::now();
}

void App::SetStatus(const std::string& message) {
  state_.status_message = message;
  state_.status_time = std::chrono::steady_clock::now();
}

void App::Tick() {
  auto now = std::chrono::steady_clock::now();
  if (!state_.status_message.empty()) {
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - state_.status_time).count();
    if (elapsed > 3) {
      state_.status_message.clear();
    }
  }

  if (state_.timer.running && !state_.timer.paused) {
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - state_.timer.last_tick).count();
    if (elapsed > 0) {
      state_.timer.remaining_seconds -= static_cast<int>(elapsed);
      state_.timer.last_tick = now;
    }

    if (state_.timer.remaining_seconds <= 0) {
      if (state_.timer.mode == AppState::TimerState::Mode::Focus) {
        try {
          session_service_.stopActiveSession(std::time(nullptr));
        } catch (...) {
          // Best-effort; avoid crashing the UI loop.
        }
        state_.timer.mode = AppState::TimerState::Mode::Break;
        state_.timer.remaining_seconds = 5 * 60;
        state_.timer.running = true;
        state_.timer.paused = false;
        state_.timer.last_tick = now;
        SetStatus("Break started.");
      } else {
        state_.timer.mode = AppState::TimerState::Mode::Idle;
        state_.timer.remaining_seconds = 25 * 60;
        state_.timer.running = false;
        state_.timer.paused = false;
        SetStatus("Break complete.");
      }
    }
  }

  auto since_stats = std::chrono::duration_cast<std::chrono::seconds>(now - state_.stats.last_update).count();
  if (since_stats >= 10) {
    RefreshStats();
  }
}
