#pragma once

#include "models.hpp"

#include <string>
#include <vector>

struct TimeBlock {
  std::string start;
  std::string end;
};

struct ScheduleItem {
  std::string start;
  std::string end;
  int task_id = 0;
  std::string title;
  std::string tag;
  std::string reason;
};

class Scheduler {
 public:
  std::vector<ScheduleItem> build_day_schedule(const std::string& date,
                                               const std::string& day_start,
                                               const std::string& day_end,
                                               const std::vector<Task>& tasks,
                                               const std::vector<Preference>& preferences,
                                               const std::vector<BusyBlock>& busy_blocks);
};
