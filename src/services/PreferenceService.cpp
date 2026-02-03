#include "services/PreferenceService.hpp"

#include <sqlite3.h>

int PreferenceService::addPreference(const Preference& pref) {
  auto stmt = db_.prepare(
      "INSERT INTO preferences(name, tag, energy, start_time, end_time, weight) "
      "VALUES(?, ?, ?, ?, ?, ?)");
  sqlite3_bind_text(stmt.get(), 1, pref.name.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 2, pref.tag.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 3, pref.energy.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 4, pref.start_time.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 5, pref.end_time.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt.get(), 6, pref.weight);

  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw DbError("Failed to insert preference: " + std::string(sqlite3_errmsg(db_.handle())));
  }
  return static_cast<int>(sqlite3_last_insert_rowid(db_.handle()));
}

std::vector<Preference> PreferenceService::listPreferences() {
  std::vector<Preference> prefs;
  auto stmt = db_.prepare(
      "SELECT id, name, tag, energy, start_time, end_time, weight FROM preferences ORDER BY id");

  auto col_text = [&](int col) -> std::string {
    const unsigned char* text = sqlite3_column_text(stmt.get(), col);
    return text ? reinterpret_cast<const char*>(text) : "";
  };

  while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
    Preference p;
    p.id = sqlite3_column_int(stmt.get(), 0);
    p.name = col_text(1);
    p.tag = col_text(2);
    p.energy = col_text(3);
    p.start_time = col_text(4);
    p.end_time = col_text(5);
    p.weight = sqlite3_column_int(stmt.get(), 6);
    prefs.push_back(std::move(p));
  }
  return prefs;
}
