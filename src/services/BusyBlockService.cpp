#include "services/BusyBlockService.hpp"

#include <sqlite3.h>

int BusyBlockService::addBusyBlock(const BusyBlock& block) {
  auto stmt = db_.prepare(
      "INSERT INTO busy_blocks(date, start_time, end_time, title) VALUES(?, ?, ?, ?)");
  sqlite3_bind_text(stmt.get(), 1, block.date.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 2, block.start_time.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 3, block.end_time.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 4, block.title.c_str(), -1, SQLITE_TRANSIENT);

  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw DbError("Failed to insert busy block: " + std::string(sqlite3_errmsg(db_.handle())));
  }
  return static_cast<int>(sqlite3_last_insert_rowid(db_.handle()));
}

std::vector<BusyBlock> BusyBlockService::listBusyBlocksForDate(const std::string& date) {
  std::vector<BusyBlock> blocks;
  auto stmt = db_.prepare(
      "SELECT id, date, start_time, end_time, title FROM busy_blocks WHERE date = ? ORDER BY start_time");
  sqlite3_bind_text(stmt.get(), 1, date.c_str(), -1, SQLITE_TRANSIENT);

  auto col_text = [&](int col) -> std::string {
    const unsigned char* text = sqlite3_column_text(stmt.get(), col);
    return text ? reinterpret_cast<const char*>(text) : "";
  };

  while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
    BusyBlock b;
    b.id = sqlite3_column_int(stmt.get(), 0);
    b.date = col_text(1);
    b.start_time = col_text(2);
    b.end_time = col_text(3);
    b.title = col_text(4);
    blocks.push_back(std::move(b));
  }
  return blocks;
}
