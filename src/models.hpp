#pragma once

#include <string>
#include <ctime>

struct Task {
  int id;
  std::string title;
  std::string tag;
  std::string due_date;
  bool completed;
  int estimate_minutes = 30;
  int priority = 3;
  int min_chunk = 30;
  bool splittable = true;
  std::string energy = "med";
};

struct Session {
  int id;
  std::string tag;
  std::time_t start_time;
  std::time_t end_time;
};

struct Preference {
  int id;
  std::string name;
  std::string tag;
  std::string energy;
  std::string start_time;  // HH:MM
  std::string end_time;    // HH:MM
  int weight = 1;
};

struct BusyBlock {
  int id;
  std::string date;        // YYYY-MM-DD
  std::string start_time;  // HH:MM
  std::string end_time;    // HH:MM
  std::string title;
};
