#pragma once

#include "models.hpp"
#include "storage/Database.hpp"

#include <string>
#include <vector>

class BusyBlockService {
 public:
  explicit BusyBlockService(Database& db) : db_(db) {}

  int addBusyBlock(const BusyBlock& block);
  std::vector<BusyBlock> listBusyBlocksForDate(const std::string& date);

 private:
  Database& db_;
};
