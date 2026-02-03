#pragma once

#include <sqlite3.h>

#include <stdexcept>
#include <string>

class Statement {
 public:
  explicit Statement(sqlite3_stmt* stmt) : stmt_(stmt) {}
  ~Statement();

  Statement(const Statement&) = delete;
  Statement& operator=(const Statement&) = delete;

  Statement(Statement&& other) noexcept;
  Statement& operator=(Statement&& other) noexcept;

  sqlite3_stmt* get() const { return stmt_; }

 private:
  sqlite3_stmt* stmt_ = nullptr;
};

class Database {
 public:
  explicit Database(const std::string& path);
  ~Database();

  Database(const Database&) = delete;
  Database& operator=(const Database&) = delete;

  void exec(const std::string& sql);
  Statement prepare(const std::string& sql);
  void initSchema();

  sqlite3* handle() const { return db_; }

 private:
  sqlite3* db_ = nullptr;
};

class DbError : public std::runtime_error {
 public:
  explicit DbError(const std::string& message) : std::runtime_error(message) {}
};
