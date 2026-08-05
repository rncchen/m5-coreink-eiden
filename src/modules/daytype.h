#pragma once
#include <string>
#include <utility>
#include <vector>
#include "types.h"

namespace railboard {

DayType classifyDay(const Time& t,
                    const std::vector<std::string>& holidays,
                    const std::pair<std::string, std::string>& holiday_range);

}  // namespace railboard
