#include "services/TaskService.hpp"

#include <sqlite3.h>

int TaskService::addTask(const std::string& title, const std::string& tag, const std::string& due_date,
                         int estimate_minutes, int priority, int min_chunk, bool splittable,
                         const std::string& energy) {
  auto stmt = db_.prepare(
      "INSERT INTO tasks(title, tag, due_date, completed, estimate_minutes, priority, min_chunk, "
      "splittable, energy) VALUES(?, ?, ?, 0, ?, ?, ?, ?, ?)");
  sqlite3_bind_text(stmt.get(), 1, title.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 2, tag.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 3, due_date.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt.get(), 4, estimate_minutes);
  sqlite3_bind_int(stmt.get(), 5, priority);
  sqlite3_bind_int(stmt.get(), 6, min_chunk);
  sqlite3_bind_int(stmt.get(), 7, splittable ? 1 : 0);
  sqlite3_bind_text(stmt.get(), 8, energy.c_str(), -1, SQLITE_TRANSIENT);

  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw DbError("Failed to insert task: " + std::string(sqlite3_errmsg(db_.handle())));
  }
  return static_cast<int>(sqlite3_last_insert_rowid(db_.handle()));
}

std::vector<Task> TaskService::listTasks() {
  std::vector<Task> tasks;
  auto stmt = db_.prepare(
      "SELECT id, title, tag, due_date, completed, estimate_minutes, priority, min_chunk, "
      "splittable, energy FROM tasks ORDER BY completed, id");

  auto col_text = [&](int col) -> std::string {
    const unsigned char* text = sqlite3_column_text(stmt.get(), col);
    return text ? reinterpret_cast<const char*>(text) : "";
  };

  while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
    Task t;
    t.id = sqlite3_column_int(stmt.get(), 0);
    t.title = col_text(1);
    t.tag = col_text(2);
    t.due_date = col_text(3);
    t.completed = sqlite3_column_int(stmt.get(), 4) != 0;
    t.estimate_minutes = sqlite3_column_int(stmt.get(), 5);
    t.priority = sqlite3_column_int(stmt.get(), 6);
    t.min_chunk = sqlite3_column_int(stmt.get(), 7);
    t.splittable = sqlite3_column_int(stmt.get(), 8) != 0;
    t.energy = col_text(9);
    tasks.push_back(std::move(t));
  }
  return tasks;
}

void TaskService::markDone(int id) {
  auto stmt = db_.prepare("UPDATE tasks SET completed = 1 WHERE id = ?");
  sqlite3_bind_int(stmt.get(), 1, id);
  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw DbError("Failed to update task: " + std::string(sqlite3_errmsg(db_.handle())));
  }
  if (sqlite3_changes(db_.handle()) == 0) {
    throw DbError("No task found with id " + std::to_string(id));
  }
}
