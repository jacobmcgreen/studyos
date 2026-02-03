#pragma once

#include "models.hpp"
#include "storage/Database.hpp"

#include <optional>
#include <utility>
#include <vector>
#include <string>

class SessionService {
 public:
  explicit SessionService(Database& db) : db_(db) {}

  int startSession(const std::string& tag, std::time_t start_time);
  void stopActiveSession(std::time_t end_time);
  std::optional<Session> getActiveSession();

  long long totalSecondsForRange(std::time_t start, std::time_t end);
  std::vector<std::pair<std::string, long long>> topTagsForRange(std::time_t start, std::time_t end,
                                                                 int limit);

 private:
  Database& db_;
};
