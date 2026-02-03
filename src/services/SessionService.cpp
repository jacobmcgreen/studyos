#include "services/SessionService.hpp"

#include <sqlite3.h>

int SessionService::startSession(const std::string& tag, std::time_t start_time) {
  if (getActiveSession().has_value()) {
    throw DbError("A session is already active. Stop it before starting a new one.");
  }
  auto stmt = db_.prepare(
      "INSERT INTO sessions(tag, start_time, end_time) VALUES(?, ?, 0)");
  sqlite3_bind_text(stmt.get(), 1, tag.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt.get(), 2, static_cast<sqlite3_int64>(start_time));

  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw DbError("Failed to insert session: " + std::string(sqlite3_errmsg(db_.handle())));
  }
  return static_cast<int>(sqlite3_last_insert_rowid(db_.handle()));
}

void SessionService::stopActiveSession(std::time_t end_time) {
  auto active = getActiveSession();
  if (!active.has_value()) {
    throw DbError("No active session to stop.");
  }
  auto stmt = db_.prepare("UPDATE sessions SET end_time = ? WHERE id = ?");
  sqlite3_bind_int64(stmt.get(), 1, static_cast<sqlite3_int64>(end_time));
  sqlite3_bind_int(stmt.get(), 2, active->id);

  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw DbError("Failed to stop session: " + std::string(sqlite3_errmsg(db_.handle())));
  }
}

std::optional<Session> SessionService::getActiveSession() {
  auto stmt = db_.prepare(
      "SELECT id, tag, start_time, end_time FROM sessions WHERE end_time = 0 "
      "ORDER BY start_time DESC LIMIT 1");

  if (sqlite3_step(stmt.get()) == SQLITE_ROW) {
    Session s;
    s.id = sqlite3_column_int(stmt.get(), 0);
    const unsigned char* text = sqlite3_column_text(stmt.get(), 1);
    s.tag = text ? reinterpret_cast<const char*>(text) : "";
    s.start_time = static_cast<std::time_t>(sqlite3_column_int64(stmt.get(), 2));
    s.end_time = static_cast<std::time_t>(sqlite3_column_int64(stmt.get(), 3));
    return s;
  }
  return std::nullopt;
}

long long SessionService::totalSecondsForRange(std::time_t start, std::time_t end) {
  auto stmt = db_.prepare(
      "SELECT SUM(end_time - start_time) FROM sessions "
      "WHERE end_time > 0 AND start_time >= ? AND start_time < ?");
  sqlite3_bind_int64(stmt.get(), 1, static_cast<sqlite3_int64>(start));
  sqlite3_bind_int64(stmt.get(), 2, static_cast<sqlite3_int64>(end));

  long long total = 0;
  if (sqlite3_step(stmt.get()) == SQLITE_ROW) {
    if (sqlite3_column_type(stmt.get(), 0) != SQLITE_NULL) {
      total = sqlite3_column_int64(stmt.get(), 0);
    }
  }
  return total;
}

std::vector<std::pair<std::string, long long>> SessionService::topTagsForRange(
    std::time_t start, std::time_t end, int limit) {
  std::vector<std::pair<std::string, long long>> results;
  auto stmt = db_.prepare(
      "SELECT tag, SUM(end_time - start_time) AS total_seconds "
      "FROM sessions WHERE end_time > 0 AND start_time >= ? AND start_time < ? "
      "GROUP BY tag ORDER BY total_seconds DESC LIMIT ?");
  sqlite3_bind_int64(stmt.get(), 1, static_cast<sqlite3_int64>(start));
  sqlite3_bind_int64(stmt.get(), 2, static_cast<sqlite3_int64>(end));
  sqlite3_bind_int(stmt.get(), 3, limit);

  while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
    const unsigned char* text = sqlite3_column_text(stmt.get(), 0);
    std::string tag = text ? reinterpret_cast<const char*>(text) : "";
    long long total = sqlite3_column_int64(stmt.get(), 1);
    results.emplace_back(tag.empty() ? "(untagged)" : tag, total);
  }
  return results;
}
