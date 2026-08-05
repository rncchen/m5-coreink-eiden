#pragma once
#include "types.h"

namespace railboard {

class RtcClock {
public:
    void begin();
    Time now();
    void set(const Time& t);
    bool isSet();
};

}  // namespace railboard
