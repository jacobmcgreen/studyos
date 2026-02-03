#include "scheduler/Scheduler.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <sstream>
#include <ctime>

namespace {

int to_minutes(const std::string& hhmm) {
  if (hhmm.size() < 4) return 0;
  int hour = std::stoi(hhmm.substr(0, 2));
  int minute = std::stoi(hhmm.substr(3, 2));
  return hour * 60 + minute;
}

std::string to_hhmm(int minutes) {
  int hour = minutes / 60;
  int minute = minutes % 60;
  std::ostringstream oss;
  oss << (hour < 10 ? "0" : "") << hour << ":" << (minute < 10 ? "0" : "") << minute;
  return oss.str();
}

int round_up_30(int minutes) {
  return ((minutes + 29) / 30) * 30;
}

int days_between(const std::string& from, const std::string& to) {
  if (from.size() != 10 || to.size() != 10) return 9999;
  int fy = std::stoi(from.substr(0, 4));
  int fm = std::stoi(from.substr(5, 2));
  int fd = std::stoi(from.substr(8, 2));
  int ty = std::stoi(to.substr(0, 4));
  int tm = std::stoi(to.substr(5, 2));
  int td = std::stoi(to.substr(8, 2));

  std::tm f = {};
  f.tm_year = fy - 1900;
  f.tm_mon = fm - 1;
  f.tm_mday = fd;
  std::tm t = {};
  t.tm_year = ty - 1900;
  t.tm_mon = tm - 1;
  t.tm_mday = td;

  std::time_t ft = std::mktime(&f);
  std::time_t tt = std::mktime(&t);
  if (ft == -1 || tt == -1) return 9999;
  double diff = std::difftime(tt, ft);
  return static_cast<int>(diff / (60 * 60 * 24));
}

std::string energy_for_time(int minutes) {
  if (minutes >= 9 * 60 && minutes < 12 * 60) return "high";
  if (minutes >= 12 * 60 && minutes < 16 * 60) return "med";
  if (minutes >= 16 * 60 && minutes < 19 * 60) return "low";
  return "med";
}

struct TaskState {
  Task task;
  int remaining;
};

}  // namespace

std::vector<ScheduleItem> Scheduler::build_day_schedule(
    const std::string& date, const std::string& day_start, const std::string& day_end,
    const std::vector<Task>& tasks, const std::vector<Preference>& preferences,
    const std::vector<BusyBlock>& busy_blocks) {
  int start_min = to_minutes(day_start);
  int end_min = to_minutes(day_end);
  if (end_min <= start_min) return {};

  int slots = (end_min - start_min) / 30;
  std::vector<bool> available(slots, true);

  for (const auto& block : busy_blocks) {
    if (block.date != date) continue;
    int bstart = to_minutes(block.start_time);
    int bend = to_minutes(block.end_time);
    int s = std::max(0, (bstart - start_min) / 30);
    int e = std::min(slots, (bend - start_min + 29) / 30);
    for (int i = s; i < e; ++i) {
      available[i] = false;
    }
  }

  std::vector<TaskState> queue;
  for (const auto& task : tasks) {
    if (task.completed) continue;
    TaskState state;
    state.task = task;
    if (!task.splittable) {
      state.remaining = round_up_30(task.estimate_minutes);
    } else {
      state.remaining = std::max(30, round_up_30(task.estimate_minutes));
    }
    queue.push_back(std::move(state));
  }

  std::vector<ScheduleItem> schedule;
  std::string last_tag;

  for (int i = 0; i < slots; ++i) {
    if (!available[i]) {
      continue;
    }

    int slot_start = start_min + i * 30;
    std::string slot_energy = energy_for_time(slot_start);

    int best_index = -1;
    double best_score = -1e9;
    std::string best_reason;
    int best_block = 30;

    for (size_t t = 0; t < queue.size(); ++t) {
      auto& entry = queue[t];
      if (entry.remaining <= 0) continue;

      int chunk = entry.task.min_chunk;
      if (!entry.task.splittable) {
        chunk = round_up_30(entry.task.estimate_minutes);
      }
      int slots_needed = chunk / 30;
      if (i + slots_needed > slots) continue;
      bool block_free = true;
      for (int s = 0; s < slots_needed; ++s) {
        if (!available[i + s]) {
          block_free = false;
          break;
        }
      }
      if (!block_free) continue;

      double score = entry.task.priority * 10.0;
      std::ostringstream reason;
      reason << "priority " << entry.task.priority;

      if (!entry.task.due_date.empty()) {
        int days = days_between(date, entry.task.due_date);
        if (days <= 0) {
          score += 50;
          reason << ", due now";
        } else if (days <= 3) {
          score += 30;
          reason << ", due in " << days << "d";
        } else if (days <= 7) {
          score += 15;
          reason << ", due in " << days << "d";
        } else if (days <= 14) {
          score += 5;
          reason << ", due in " << days << "d";
        }
      }

      if (!entry.task.energy.empty() && entry.task.energy == slot_energy) {
        score += 6;
        reason << ", energy match";
      }

      for (const auto& pref : preferences) {
        int pstart = to_minutes(pref.start_time);
        int pend = to_minutes(pref.end_time);
        if (slot_start < pstart || slot_start >= pend) {
          continue;
        }
        bool match = false;
        if (!pref.tag.empty() && pref.tag == entry.task.tag) {
          match = true;
        }
        if (!pref.energy.empty() && pref.energy == entry.task.energy) {
          match = true;
        }
        if (match) {
          score += pref.weight;
          if (!pref.name.empty()) {
            reason << ", pref " << pref.name;
          } else {
            reason << ", pref match";
          }
        }
      }

      if (!last_tag.empty() && entry.task.tag != last_tag) {
        score -= 2.0;
        reason << ", switch penalty";
      }

      if (score > best_score) {
        best_score = score;
        best_index = static_cast<int>(t);
        best_reason = reason.str();
        best_block = chunk;
      }
    }

    if (best_index == -1) {
      continue;
    }

    auto& chosen = queue[static_cast<size_t>(best_index)];
    ScheduleItem item;
    item.start = to_hhmm(slot_start);
    item.end = to_hhmm(slot_start + best_block);
    item.task_id = chosen.task.id;
    item.title = chosen.task.title;
    item.tag = chosen.task.tag;
    item.reason = best_reason;
    schedule.push_back(item);

    int slots_used = best_block / 30;
    for (int s = 0; s < slots_used && i + s < slots; ++s) {
      available[i + s] = false;
    }

    chosen.remaining -= best_block;
    if (chosen.remaining < 0) chosen.remaining = 0;
    last_tag = chosen.task.tag;
  }

  return schedule;
}
