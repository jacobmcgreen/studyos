#pragma once

#include "models.hpp"

#include <chrono>
#include <string>
#include <utility>
#include <vector>

struct AppState {
  struct TimerState {
    enum class Mode { Idle, Focus, Break };
    Mode mode = Mode::Idle;
    int remaining_seconds = 25 * 60;
    int total_seconds = 25 * 60;
    bool running = false;
    bool paused = false;
    bool session_active = false;
    int session_count = 0;
    std::string last_tag;
    std::chrono::steady_clock::time_point last_tick = std::chrono::steady_clock::now();
  } timer;

  struct TasksState {
    std::vector<Task> tasks;
    std::string filter;
    int selected_index = 0;
  } tasks;

  struct StatsState {
    long long today_seconds = 0;
    long long week_seconds = 0;
    std::vector<std::pair<std::string, long long>> top_tags;
    std::vector<std::pair<std::string, long long>> today_tags;
    std::chrono::steady_clock::time_point last_update = std::chrono::steady_clock::now();
  } stats;

  int tab_index = 0;
};
