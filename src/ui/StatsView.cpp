#include "ui/StatsView.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <algorithm>

namespace {

std::string FormatMinutes(long long seconds) {
  long long minutes = seconds / 60;
  return std::to_string(minutes) + " min";
}

ftxui::Element Bar(long long value, long long max_value, int width) {
  using namespace ftxui;
  int count = 0;
  if (max_value > 0) {
    count = static_cast<int>(value * width / max_value);
  }
  std::string bar(count, '#');
  bar.resize(width, ' ');
  return text(bar);
}

}  // namespace

ftxui::Component MakeStatsView(AppState& state, SessionService& session_service,
                               std::function<void()> refresh_stats) {
  using namespace ftxui;

  (void)session_service;
  (void)refresh_stats;

  auto renderer = Renderer([&] {
    long long max_value = 0;
    for (const auto& pair : state.stats.top_tags) {
      max_value = std::max(max_value, pair.second);
    }

    Elements rows;
    for (const auto& pair : state.stats.top_tags) {
      rows.push_back(hbox({
          text(pair.first.empty() ? "(none)" : pair.first) | size(WIDTH, EQUAL, 12),
          Bar(pair.second, max_value, 20),
          text(" " + FormatMinutes(pair.second)),
      }));
    }

    if (rows.empty()) {
      rows.push_back(text("No sessions yet."));
    }

    auto summary = vbox({
        text("Today: " + FormatMinutes(state.stats.today_seconds)),
        text("This week: " + FormatMinutes(state.stats.week_seconds)),
    });

    return vbox({
        text("Stats") | bold,
        separator(),
        summary | border,
        text("Top Tags") | bold,
        vbox(rows) | border,
    });
  });

  return renderer;
}
