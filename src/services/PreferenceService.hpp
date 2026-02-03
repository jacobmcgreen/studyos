#pragma once

#include "models.hpp"
#include "storage/Database.hpp"

#include <vector>

class PreferenceService {
 public:
  explicit PreferenceService(Database& db) : db_(db) {}

  int addPreference(const Preference& pref);
  std::vector<Preference> listPreferences();

 private:
  Database& db_;
};
