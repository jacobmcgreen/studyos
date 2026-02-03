#include "ui/TimerView.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include <chrono>
#include <ctime>
#include <sstream>

namespace {

std::string ModeLabel(AppState::TimerState::Mode mode) {
  switch (mode) {
    case AppState::TimerState::Mode::Idle:
      return "IDLE";
    case AppState::TimerState::Mode::Focus:
      return "FOCUS";
    case AppState::TimerState::Mode::Break:
      return "BREAK";
  }
  return "";
}

std::string FormatTimer(int seconds) {
  int m = seconds / 60;
  int s = seconds % 60;
  std::ostringstream oss;
  oss << m << "m " << s << "s";
  return oss.str();
}

}  // namespace

ftxui::Component MakeTimerView(AppState& state, SessionService& session_service,
                               std::function<void(const std::string&)> set_status) {
  using namespace ftxui;

  bool show_start_modal = false;
  std::string tag_input;

  auto input = Input(&tag_input, "tag");
  auto start_button = Button("Start", [&] {
    if (tag_input.empty()) {
      set_status("Tag required to start.");
      return;
    }
    try {
      auto now = std::time(nullptr);
      session_service.startSession(tag_input, now);
      state.timer.last_tag = tag_input;
      state.timer.mode = AppState::TimerState::Mode::Focus;
      state.timer.remaining_seconds = 25 * 60;
      state.timer.running = true;
      state.timer.paused = false;
      state.timer.last_tick = std::chrono::steady_clock::now();
      show_start_modal = false;
      tag_input.clear();
      set_status("Focus session started.");
    } catch (const std::exception& ex) {
      set_status(ex.what());
    }
  });
  auto cancel_button = Button("Cancel", [&] { show_start_modal = false; });

  auto modal = Container::Vertical({input, start_button, cancel_button});

  auto renderer = Renderer([&] {
    auto timer_box = vbox({
        text("Mode: " + ModeLabel(state.timer.mode)),
        text("Remaining: " + FormatTimer(state.timer.remaining_seconds)),
        text(state.timer.running ? (state.timer.paused ? "Paused" : "Running") : "Stopped"),
        text(state.timer.last_tag.empty() ? "Tag: (none)" : "Tag: " + state.timer.last_tag),
    });

    auto body = vbox({
        text("Timer") | bold,
        separator(),
        timer_box | border,
        text("Keys: s start, p pause/resume, x stop, r reset"),
    });

    if (show_start_modal) {
      auto modal_box = vbox({
          text("Start Focus Session") | bold,
          text("Tag:"),
          input->Render(),
          hbox({start_button->Render(), text(" "), cancel_button->Render()}),
      }) | border | bgcolor(Color::GrayDark);
      return dbox({body, modal_box | center});
    }

    return body;
  });

  auto component = CatchEvent(renderer, [&](Event event) {
    if (event == Event::Character('s')) {
      if (state.timer.running) {
        set_status("Session already running.");
        return true;
      }
      if (!state.timer.last_tag.empty()) {
        try {
          auto now = std::time(nullptr);
          session_service.startSession(state.timer.last_tag, now);
          state.timer.mode = AppState::TimerState::Mode::Focus;
          state.timer.remaining_seconds = 25 * 60;
          state.timer.running = true;
          state.timer.paused = false;
          state.timer.last_tick = std::chrono::steady_clock::now();
          set_status("Focus session started (last tag).");
        } catch (const std::exception& ex) {
          set_status(ex.what());
        }
      } else {
        show_start_modal = true;
      }
      return true;
    }
    if (event == Event::Character('p')) {
      if (!state.timer.running) {
        set_status("No active timer.");
        return true;
      }
      state.timer.paused = !state.timer.paused;
      set_status(state.timer.paused ? "Paused." : "Resumed.");
      return true;
    }
    if (event == Event::Character('x')) {
      if (!state.timer.running) {
        set_status("No active session.");
        return true;
      }
      try {
        session_service.stopActiveSession(std::time(nullptr));
        state.timer.running = false;
        state.timer.paused = false;
        state.timer.mode = AppState::TimerState::Mode::Idle;
        state.timer.remaining_seconds = 25 * 60;
        set_status("Session stopped.");
      } catch (const std::exception& ex) {
        set_status(ex.what());
      }
      return true;
    }
    if (event == Event::Character('r')) {
      if (state.timer.running) {
        set_status("Stop session before reset.");
        return true;
      }
      state.timer.remaining_seconds = 25 * 60;
      state.timer.mode = AppState::TimerState::Mode::Idle;
      state.timer.running = false;
      state.timer.paused = false;
      set_status("Timer reset.");
      return true;
    }
    return false;
  });

  return Container::Vertical({modal, component});
}
