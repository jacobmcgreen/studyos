#include "ui/TasksView.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include <algorithm>

namespace {

std::vector<int> FilteredIndices(const std::vector<Task>& tasks, const std::string& filter) {
  std::vector<int> indices;
  for (size_t i = 0; i < tasks.size(); ++i) {
    if (filter.empty() || tasks[i].title.find(filter) != std::string::npos) {
      indices.push_back(static_cast<int>(i));
    }
  }
  return indices;
}

std::string TaskLine(const Task& t) {
  std::string line = t.completed ? "[x] " : "[ ] ";
  line += t.title;
  if (!t.tag.empty()) {
    line += " (" + t.tag + ")";
  }
  return line;
}

}  // namespace

ftxui::Component MakeTasksView(AppState& state, TaskService& task_service,
                               std::function<void()> refresh_tasks,
                               std::function<void(const std::string&)> set_status) {
  using namespace ftxui;

  bool show_new_modal = false;
  bool show_filter_modal = false;
  std::string title_input;
  std::string tag_input;
  std::string due_input;
  std::string filter_input;

  std::vector<std::string> entries;
  std::vector<int> filtered_indices;

  auto menu = Menu(&entries, &state.tasks.selected_index);

  auto title_field = Input(&title_input, "title");
  auto tag_field = Input(&tag_input, "tag");
  auto due_field = Input(&due_input, "YYYY-MM-DD");
  auto filter_field = Input(&filter_input, "filter");

  auto save_button = Button("Save", [&] {
    if (title_input.empty()) {
      set_status("Title required.");
      return;
    }
    try {
      task_service.addTask(title_input, tag_input, due_input);
      title_input.clear();
      tag_input.clear();
      due_input.clear();
      show_new_modal = false;
      refresh_tasks();
      set_status("Task added.");
    } catch (const std::exception& ex) {
      set_status(ex.what());
    }
  });

  auto cancel_button = Button("Cancel", [&] { show_new_modal = false; });
  auto apply_filter = Button("Apply", [&] {
    state.tasks.filter = filter_input;
    show_filter_modal = false;
    set_status("Filter updated.");
  });
  auto clear_filter = Button("Clear", [&] {
    state.tasks.filter.clear();
    filter_input.clear();
    show_filter_modal = false;
    set_status("Filter cleared.");
  });

  auto modal = Container::Vertical({title_field, tag_field, due_field, save_button, cancel_button});
  auto filter_modal = Container::Vertical({filter_field, apply_filter, clear_filter});

  auto renderer = Renderer(menu, [&] {
    filtered_indices = FilteredIndices(state.tasks.tasks, state.tasks.filter);
    if (state.tasks.selected_index >= static_cast<int>(filtered_indices.size())) {
      state.tasks.selected_index = static_cast<int>(filtered_indices.size()) - 1;
    }
    if (state.tasks.selected_index < 0) {
      state.tasks.selected_index = 0;
    }

    entries.clear();
    for (int idx : filtered_indices) {
      entries.push_back(TaskLine(state.tasks.tasks[idx]));
    }

    Element detail = text("Select a task.");
    if (!filtered_indices.empty() && state.tasks.selected_index >= 0 &&
        state.tasks.selected_index < static_cast<int>(filtered_indices.size())) {
      const auto& t = state.tasks.tasks[filtered_indices[state.tasks.selected_index]];
      detail = vbox({
          text("Title: " + t.title),
          text("Tag: " + (t.tag.empty() ? "-" : t.tag)),
          text("Due: " + (t.due_date.empty() ? "-" : t.due_date)),
          text(std::string("Status: ") + (t.completed ? "Done" : "Open")),
      });
    }

    auto body = hbox({
        vbox({text("Tasks") | bold, separator(), menu->Render() | border}) | flex,
        vbox({text("Details") | bold, separator(), detail | border}) | flex,
    });

    if (show_new_modal) {
      auto box = vbox({
          text("New Task") | bold,
          text("Title"),
          title_field->Render(),
          text("Tag"),
          tag_field->Render(),
          text("Due"),
          due_field->Render(),
          hbox({save_button->Render(), text(" "), cancel_button->Render()}),
      }) | border | bgcolor(Color::GrayDark);
      return dbox({body, box | center});
    }

    if (show_filter_modal) {
      auto box = vbox({
          text("Filter Tasks") | bold,
          filter_field->Render(),
          hbox({apply_filter->Render(), text(" "), clear_filter->Render()}),
      }) | border | bgcolor(Color::GrayDark);
      return dbox({body, box | center});
    }

    return body;
  });

  auto component = CatchEvent(renderer, [&](Event event) {
    if (event == Event::Character('n')) {
      show_new_modal = true;
      return true;
    }
    if (event == Event::Character('d')) {
      if (filtered_indices.empty() || state.tasks.selected_index >= static_cast<int>(filtered_indices.size())) {
        set_status("No task selected.");
        return true;
      }
      int idx = filtered_indices[state.tasks.selected_index];
      if (!state.tasks.tasks[idx].completed) {
        try {
          task_service.markDone(state.tasks.tasks[idx].id);
          refresh_tasks();
          set_status("Task marked done.");
        } catch (const std::exception& ex) {
          set_status(ex.what());
        }
      }
      return true;
    }
    if (event == Event::Character('/')) {
      filter_input = state.tasks.filter;
      show_filter_modal = true;
      return true;
    }
    if (event == Event::Character('j')) {
      state.tasks.selected_index += 1;
      return true;
    }
    if (event == Event::Character('k')) {
      state.tasks.selected_index -= 1;
      return true;
    }
    return false;
  });

  return Container::Vertical({modal, filter_modal, component});
}
