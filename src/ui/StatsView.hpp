#pragma once

#include "ui/App.hpp"

#include <ftxui/component/component.hpp>

#include <functional>

ftxui::Component MakeStatsView(AppState& state, SessionService& session_service,
                               std::function<void()> refresh_stats);
