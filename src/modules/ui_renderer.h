#pragma once
#include <optional>
#include <string>
#include <vector>
#include "data_loader.h"
#include "types.h"

namespace railboard {

struct RenderContext {
    const StationMeta* station;
    const StationsMeta* meta;
    Time now;
    std::vector<Departure> up_next;
    std::vector<Departure> down_next;
    std::optional<Departure> up_last;    // day's final departure (shown after service end)
    std::optional<Departure> down_last;
};

class UiRenderer {
public:
    void begin();
    // Always draws the full page with the epd_quality waveform
    // (inverted double refresh; ghosting does not accumulate)
    void renderMain(const RenderContext& ctx);
    void renderMessage(const std::string& primary, const std::string& secondary);
    // Black/white inversion cycle to clear deep long-term ghosting
    // (used by night maintenance)
    void deepClean();
    // Middle-button toggle: rotate the whole UI 180° for upside-down mounting
    void setFlipped(bool flipped);
};

}  // namespace railboard
