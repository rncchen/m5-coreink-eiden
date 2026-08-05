#pragma once

namespace railboard {

enum class ButtonEvent { None, Up, Press, Down, Top };

class ButtonInput {
public:
    ButtonEvent poll();
};

}  // namespace railboard
