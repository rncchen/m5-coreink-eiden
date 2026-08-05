#include "ui_renderer.h"
#if __has_include("fonts/local_destination_font.h")
#include "fonts/local_destination_font.h"
#define RAILBOARD_HAS_LOCAL_DEST_FONT 1
#else
#define RAILBOARD_HAS_LOCAL_DEST_FONT 0
#endif
#include <M5Unified.h>
#include <cstdio>

namespace railboard {

// Layout constants (departure-board style: black banners + large time digits)
// 28 + (20 + 2*33) * 2 = 200
constexpr int W = 200;
constexpr int HEADER_H = 28;
constexpr int DIR_HEADER_H = 20;
constexpr int ROW_H = 33;

static std::string fmtHHMM(int hhmm) {
    char b[6]; std::snprintf(b, sizeof(b), "%02d:%02d", hhmm/100, hhmm%100);
    return b;
}

// Pick the largest Japanese font size (24 -> 16 -> 12) that fits within avail,
// set it as the active font, and return the chosen size.
// Note: only already-linked sizes may be used here; adding another efont size
// costs hundreds of KB of flash (currently at 98% usage).
static int fitJaFont(const std::string& text, int avail) {
    static const std::pair<const lgfx::IFont*, int> tiers[] = {
        {&fonts::efontJA_24, 24}, {&fonts::efontJA_16, 16},
        {&fonts::efontJA_12, 12},
    };
    for (auto& [f, size] : tiers) {
        M5.Display.setFont(f);
        if (M5.Display.textWidth(text.c_str()) <= avail) return size;
    }
    return 12;  // use the smallest tier even if it overflows (tolerate odd data)
}

// A deployment may keep a local 20 px destination subset font. Public clones
// fall back to the complete Japanese font tiers when that private asset is
// absent.
static int fitDestFont(const std::string& text, int avail) {
#if RAILBOARD_HAS_LOCAL_DEST_FONT
    static const std::pair<const lgfx::IFont*, int> tiers[] = {
        {&fonts::efontJA_24, 24}, {&localDestinationFont, 20},
        {&fonts::efontJA_16, 16}, {&fonts::efontJA_12, 12},
    };
    for (auto& [font, size] : tiers) {
        M5.Display.setFont(font);
        if (M5.Display.textWidth(text.c_str()) <= avail) return size;
    }
    return 12;
#else
    return fitJaFont(text, avail);
#endif
}

void UiRenderer::begin() {
    M5.Display.setEpdMode(epd_mode_t::epd_fastest);
    M5.Display.setRotation(0);
}

void UiRenderer::setFlipped(bool flipped) {
    // M5 CoreInk 200x200 e-ink: rotation 0 = upright, 2 = 180° flipped.
    // The caller is responsible for refreshing the screen afterwards.
    M5.Display.setRotation(flipped ? 2 : 0);
}

void UiRenderer::deepClean() {
    M5.Display.setEpdMode(epd_mode_t::epd_quality);
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.display();
    M5.Display.fillScreen(TFT_WHITE);
    M5.Display.display();
    M5.Display.setEpdMode(epd_mode_t::epd_fastest);
}

static void drawHeader(const RenderContext& ctx) {
    M5.Display.fillRect(0, 0, W, HEADER_H, TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    const std::string& name = ctx.station->name_ja;
    int size = fitJaFont(name, W - 4);
    M5.Display.setCursor(2, size == 24 ? 2 : 7);
    M5.Display.print(name.c_str());
    M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);
}

static void drawDirection(int y, const char* arrow, const std::string& label,
                          const std::vector<Departure>& deps,
                          const std::optional<Departure>& last,
                          const StationsMeta& meta) {
    // Direction header: white-on-black banner
    M5.Display.fillRect(0, y, W, DIR_HEADER_H, TFT_BLACK);
    M5.Display.setFont(&fonts::efontJA_16);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(2, y + 2);
    M5.Display.printf("%s %s", arrow, label.c_str());
    M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);

    for (size_t i = 0; i < deps.size() && i < 2; ++i) {
        int ry = y + DIR_HEADER_H + i * ROW_H;
        auto& d = deps[i];
        // Font4 (26 px tall, narrow digits): saves ~28 px of width for the
        // destination compared to FreeSansBold18pt
        // Font4's digit ink sits high in its 26 px cell; +6 keeps the digits
        // visually centered against the taller destination glyphs
        M5.Display.setFont(&fonts::Font4);
        M5.Display.setCursor(2, ry + 6);
        M5.Display.print(fmtHHMM(d.hhmm).c_str());
        int dest_x = 2 + M5.Display.textWidth(fmtHHMM(d.hhmm).c_str()) + 8;
        uint8_t v = static_cast<uint8_t>(d.vehicle);
        bool badge = (v != 0 && v < meta.vehicle_badges.size());
        if (d.dest_index >= 0 &&
            static_cast<size_t>(d.dest_index) < meta.dest_labels.size()) {
            const std::string& dest = meta.dest_labels[d.dest_index];
            // Full destination name, no abbreviation: if 24 px does not fit
            // (badge space included), step down by measured width
            int avail = (badge ? W - 28 : W - 4) - dest_x;
            int size = fitDestFont(dest, avail);
            M5.Display.setCursor(dest_x, ry + 4 + (24 - size) / 2);
            M5.Display.print(dest.c_str());
        }
        if (badge) {
            int bx = W - 24, by = ry + 5;
            M5.Display.fillRect(bx, by, 22, 22, TFT_BLACK);
            M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
            M5.Display.setFont(&fonts::efontJA_16);
            M5.Display.setCursor(bx + 7, by + 3);
            M5.Display.print(meta.vehicle_badges[v].c_str());
            M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);
        }
    }
    if (deps.empty()) {
        M5.Display.setFont(&fonts::efontJA_24);
        M5.Display.setCursor(52, y + DIR_HEADER_H + 6);
        M5.Display.print("本日終電");
        if (last) {  // after service end, show the day's final departure
            std::string txt = "終発 " + fmtHHMM(last->hhmm);
            if (last->dest_index >= 0 &&
                static_cast<size_t>(last->dest_index) < meta.dest_labels.size())
                txt += " " + meta.dest_labels[last->dest_index];
            fitJaFont(txt, W - 8);
            M5.Display.setCursor(4, y + DIR_HEADER_H + 38);
            M5.Display.print(txt.c_str());
        }
    }
}

void UiRenderer::renderMain(const RenderContext& ctx) {
    // display() in epd_quality mode runs the inverted double refresh; the
    // only waveform on this panel that resets charge and clears ghosting.
    // Wakes are down to ~100+ per day and happen right as a train departs, so
    // every redraw can afford the full waveform and ghosting never
    // accumulates; epd_fastest is reserved for transient messages.
    M5.Display.setEpdMode(epd_mode_t::epd_quality);
    M5.Display.startWrite();
    M5.Display.fillScreen(TFT_WHITE);
    drawHeader(ctx);
    // The upper half shows down; the lower half shows up.
    const int lower_y = HEADER_H + DIR_HEADER_H + 2*ROW_H;
    if (ctx.station->directions.size() >= 2) {
        drawDirection(HEADER_H, "▼", ctx.station->directions[1].label,
                      ctx.down_next, ctx.down_last, *ctx.meta);
        drawDirection(lower_y, "▲", ctx.station->directions[0].label,
                      ctx.up_next, ctx.up_last, *ctx.meta);
    } else if (ctx.station->directions.size() == 1) {
        // A terminal keeps the same up/down placement as a two-direction stop.
        const auto& dir = ctx.station->directions[0];
        bool data_is_up = dir.id == "up";
        bool display_is_up = ctx.station->display_direction.empty()
            ? data_is_up : ctx.station->display_direction == "up";
        int dir_y  = display_is_up ? lower_y : HEADER_H;
        int term_y = display_is_up ? HEADER_H : lower_y;
        drawDirection(dir_y, display_is_up ? "▲" : "▼", dir.label,
                      data_is_up ? ctx.up_next : ctx.down_next,
                      data_is_up ? ctx.up_last : ctx.down_last, *ctx.meta);
        M5.Display.setFont(&fonts::efontJA_24);
        M5.Display.setCursor(52, term_y + 40);
        M5.Display.print("終着駅");
    }
    M5.Display.endWrite();
    M5.Display.display();
    M5.Display.setEpdMode(epd_mode_t::epd_fastest);
}

void UiRenderer::renderMessage(const std::string& primary, const std::string& secondary) {
    M5.Display.fillScreen(TFT_WHITE);
    M5.Display.setFont(&fonts::efontJA_24);
    M5.Display.setCursor(10, 60);
    M5.Display.print(primary.c_str());
    M5.Display.setCursor(10, 100);
    M5.Display.print(secondary.c_str());
    M5.Display.display();
}

}  // namespace railboard
