#pragma once

#include "ui/App.hpp"

#include <ftxui/component/component.hpp>

#include <functional>

ftxui::Component MakeTimerView(AppState& state, SessionService& session_service,
                               std::function<void(const std::string&)> set_status);
