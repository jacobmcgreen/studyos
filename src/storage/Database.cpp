#include "storage/Database.hpp"

#include <utility>

Statement::~Statement() {
  if (stmt_) {
    sqlite3_finalize(stmt_);
  }
}

Statement::Statement(Statement&& other) noexcept : stmt_(other.stmt_) {
  other.stmt_ = nullptr;
}

Statement& Statement::operator=(Statement&& other) noexcept {
  if (this != &other) {
    if (stmt_) {
      sqlite3_finalize(stmt_);
    }
    stmt_ = other.stmt_;
    other.stmt_ = nullptr;
  }
  return *this;
}

Database::Database(const std::string& path) {
  if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
    std::string msg = sqlite3_errmsg(db_ ? db_ : nullptr);
    if (db_) {
      sqlite3_close(db_);
    }
    throw DbError("Failed to open database: " + msg);
  }
}

Database::~Database() {
  if (db_) {
    sqlite3_close(db_);
  }
}

void Database::exec(const std::string& sql) {
  char* err = nullptr;
  if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
    std::string msg = err ? err : "unknown error";
    sqlite3_free(err);
    throw DbError("SQLite exec error: " + msg);
  }
}

Statement Database::prepare(const std::string& sql) {
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw DbError("SQLite prepare error: " + std::string(sqlite3_errmsg(db_)));
  }
  return Statement(stmt);
}

namespace {

bool column_exists(sqlite3* db, const std::string& table, const std::string& column) {
  std::string sql = "PRAGMA table_info(" + table + ");";
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    return false;
  }
  bool found = false;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* text = sqlite3_column_text(stmt, 1);
    if (text && column == reinterpret_cast<const char*>(text)) {
      found = true;
      break;
    }
  }
  sqlite3_finalize(stmt);
  return found;
}

}  // namespace

void Database::initSchema() {
  exec(
      "CREATE TABLE IF NOT EXISTS tasks("
      "id INTEGER PRIMARY KEY,"
      "title TEXT,"
      "tag TEXT,"
      "due_date TEXT,"
      "completed INTEGER,"
      "estimate_minutes INTEGER NOT NULL DEFAULT 30,"
      "priority INTEGER NOT NULL DEFAULT 3,"
      "min_chunk INTEGER NOT NULL DEFAULT 30,"
      "splittable INTEGER NOT NULL DEFAULT 1,"
      "energy TEXT NOT NULL DEFAULT 'med'"
      ");");

  if (!column_exists(db_, "tasks", "estimate_minutes")) {
    exec("ALTER TABLE tasks ADD COLUMN estimate_minutes INTEGER NOT NULL DEFAULT 30;");
  }
  if (!column_exists(db_, "tasks", "priority")) {
    exec("ALTER TABLE tasks ADD COLUMN priority INTEGER NOT NULL DEFAULT 3;");
  }
  if (!column_exists(db_, "tasks", "min_chunk")) {
    exec("ALTER TABLE tasks ADD COLUMN min_chunk INTEGER NOT NULL DEFAULT 30;");
  }
  if (!column_exists(db_, "tasks", "splittable")) {
    exec("ALTER TABLE tasks ADD COLUMN splittable INTEGER NOT NULL DEFAULT 1;");
  }
  if (!column_exists(db_, "tasks", "energy")) {
    exec("ALTER TABLE tasks ADD COLUMN energy TEXT NOT NULL DEFAULT 'med';");
  }

  exec(
      "CREATE TABLE IF NOT EXISTS sessions("
      "id INTEGER PRIMARY KEY,"
      "tag TEXT,"
      "start_time INTEGER,"
      "end_time INTEGER"
      ");");

  exec(
      "CREATE TABLE IF NOT EXISTS preferences("
      "id INTEGER PRIMARY KEY,"
      "name TEXT,"
      "tag TEXT,"
      "energy TEXT,"
      "start_time TEXT,"
      "end_time TEXT,"
      "weight INTEGER"
      ");");

  exec(
      "CREATE TABLE IF NOT EXISTS busy_blocks("
      "id INTEGER PRIMARY KEY,"
      "date TEXT,"
      "start_time TEXT,"
      "end_time TEXT,"
      "title TEXT"
      ");");
}
