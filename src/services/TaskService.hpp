#pragma once

#include "models.hpp"
#include "storage/Database.hpp"

#include <string>
#include <vector>

class TaskService {
 public:
  explicit TaskService(Database& db) : db_(db) {}

  int addTask(const std::string& title, const std::string& tag, const std::string& due_date,
              int estimate_minutes, int priority, int min_chunk, bool splittable,
              const std::string& energy);
  std::vector<Task> listTasks();
  void markDone(int id);
  void deleteTask(int id);

 private:
  Database& db_;
};
