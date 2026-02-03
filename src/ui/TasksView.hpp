#pragma once

#include "ui/App.hpp"

#include <ftxui/component/component.hpp>

#include <functional>

ftxui::Component MakeTasksView(AppState& state, TaskService& task_service,
                               std::function<void()> refresh_tasks,
                               std::function<void(const std::string&)> set_status);
