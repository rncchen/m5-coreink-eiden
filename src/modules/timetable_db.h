#pragma once
#include <optional>
#include <vector>
#include "types.h"

namespace railboard {

std::vector<Departure> nextDepartures(const std::vector<Departure>& day_schedule,
                                      const Time& now,
                                      int count);

// The day's final departure (schedule may be unsorted; picks the max hhmm);
// nullopt for an empty schedule
std::optional<Departure> lastDeparture(const std::vector<Departure>& day_schedule);

// Minute of day when the screen content will next change: the minute after
// the earliest displayed departure across both directions leaves
// (nextDepartures compares with >=, so a train stays visible during its
// departure minute). May return 1440 (past midnight; caller takes modulo);
// nullopt when both directions are past their last train.
std::optional<int> nextWakeMinute(const std::vector<Departure>& up_schedule,
                                  const std::vector<Departure>& down_schedule,
                                  const Time& now);

}  // namespace railboard
