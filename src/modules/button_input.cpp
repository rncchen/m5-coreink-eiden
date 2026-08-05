#include "button_input.h"
#include <M5Unified.h>

namespace railboard {

ButtonEvent ButtonInput::poll() {
    M5.update();
    // Verified on a real Core Ink: BtnA is stick "down" and BtnC is "up"
    // (the opposite of what the names suggest)
    if (M5.BtnA.wasClicked())   return ButtonEvent::Down;
    if (M5.BtnB.wasClicked())   return ButtonEvent::Press;
    if (M5.BtnC.wasClicked())   return ButtonEvent::Up;
    if (M5.BtnEXT.wasClicked()) return ButtonEvent::Top;
    return ButtonEvent::None;
}

}  // namespace railboard
