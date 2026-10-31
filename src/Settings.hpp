#pragma once
// Feature registry + persistent settings. Header-only, included once from main.cpp.
#include <Geode/Geode.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <ctime>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

using namespace geode::prelude;

namespace mm {

inline bool g_menuOpen = false;     // true while ModMenuLayer is on screen
inline float g_rainbowHue = 0.f;    // animated hue (degrees) for Rainbow accent mode

enum class Kind { Toggle, Slider, Choice, Button, Info };

struct Feature {
    const char* id;
    const char* label;
    Kind kind;
    const char* window;
    float def;
    float min;
    float max;
    float step;
    std::vector<const char*> choices;
};

struct WindowDef {
    const char* id;
    const char* title;
    ccColor3B accent;
    std::vector<Feature> features;
};

// ---- registry builders -----------------------------------------------------
inline Feature T(const char* w, const char* id, const char* label, bool def = false) {
    return {id, label, Kind::Toggle, w, def ? 1.f : 0.f, 0.f, 1.f, 1.f, {}};
}
inline Feature S(const char* w, const char* id, const char* label, float def, float mn, float mx, float step) {
    return {id, label, Kind::Slider, w, def, mn, mx, step, {}};
}
inline Feature C(const char* w, const char* id, const char* label, int def, std::vector<const char*> ch) {
    return {id, label, Kind::Choice, w, (float)def, 0.f, (float)ch.size() - 1.f, 1.f, std::move(ch)};
}
inline Feature B(const char* w, const char* id, const char* label) {
    return {id, label, Kind::Button, w, 0.f, 0.f, 0.f, 1.f, {}};
}
inline Feature I(const char* w, const char* id, const char* label) {
    return {id, label, Kind::Info, w, 0.f, 0.f, 0.f, 1.f, {}};
}

inline std::vector<WindowDef> const& windowDefs() {
    static const std::vector<WindowDef> defs = {
        {"display", "Display", {70, 190, 220}, {
            T("display", "show-fps", "Show FPS (Cocos)"),
            T("display", "fps-modifier", "FPS Modifier 120"),
            T("display", "fps-custom", "Custom FPS Target"),
            S("display", "fps-target", "FPS Target", 60, 30, 240, 5),
            T("display", "fps-saver", "Battery Saver 30"),
            B("display", "fps-p30", "Set 30 FPS"),
            B("display", "fps-p60", "Set 60 FPS"),
            B("display", "fps-p90", "Set 90 FPS"),
            B("display", "fps-p120", "Set 120 FPS"),
            B("display", "fps-p144", "Set 144 FPS"),
            B("display", "fps-p240", "Set 240 FPS"),
            B("display", "fps-reset", "Reset FPS"),
            T("display", "fps-startup", "Apply FPS At Startup", true),
        }},
        {"overlay", "Stats Overlay", {255, 170, 70}, {
            T("overlay", "ov-enable", "Enable Overlay"),
            T("overlay", "ov-fps", "Show FPS", true),
            T("overlay", "ov-frametime", "Frame Time"),
            T("overlay", "ov-avg", "Average FPS"),
            T("overlay", "ov-min", "Minimum FPS"),
            T("overlay", "ov-max", "Maximum FPS"),
            T("overlay", "ov-low1", "1% Low FPS"),
            T("overlay", "ov-jitter", "Frame Jitter"),
            T("overlay", "ov-graph", "FPS Graph"),
            T("overlay", "ov-clock", "Clock"),
            T("overlay", "ov-24h", "24 Hour Clock", true),
            T("overlay", "ov-seconds", "Clock Seconds", true),
            T("overlay", "ov-date", "Date"),
            T("overlay", "ov-session", "Session Time"),
            T("overlay", "ov-frames", "Frame Counter"),
            T("overlay", "ov-res", "Resolution"),
            T("overlay", "ov-nodes", "Scene Node Count"),
            T("overlay", "ov-children", "Scene Children"),
            T("overlay", "ov-target", "Show FPS Target"),
            T("overlay", "ov-color", "Color By FPS", true),
            T("overlay", "ov-bg", "Background", true),
            T("overlay", "ov-compact", "Compact Labels"),
            T("overlay", "ov-hide-menu", "Hide While Menu Open"),
            C("overlay", "ov-corner", "Corner", 0, {"Top Left", "Top Right", "Bot Left", "Bot Right"}),
            C("overlay", "ov-rate", "Update Rate", 1, {"0.25s", "0.5s", "1s"}),
            C("overlay", "ov-decimals", "Decimals", 0, {"0", "1", "2"}),
            C("overlay", "ov-font", "Overlay Font", 0, {"Big", "Gold"}),
            S("overlay", "ov-scale", "Text Size", 0.35f, 0.2f, 1.0f, 0.05f),
            S("overlay", "ov-opacity", "Text Opacity", 100, 20, 100, 5),
            S("overlay", "ov-bgopacity", "BG Opacity", 50, 0, 100, 5),
            S("overlay", "ov-graph-h", "Graph Height", 40, 20, 120, 5),
            S("overlay", "ov-graph-w", "Graph Width", 120, 60, 260, 10),
            S("overlay", "ov-samples", "Sample Count", 120, 30, 240, 10),
            S("overlay", "ov-warn", "Warn Below FPS", 45, 10, 120, 5),
            S("overlay", "ov-pad", "Screen Margin", 4, 0, 40, 2),
            B("overlay", "ov-reset", "Reset Stats"),
        }},
        {"appearance", "Appearance", {190, 130, 255}, {
            C("appearance", "layout-preset", "Layout Preset", 0, {"Custom", "Megahack", "Eclipse", "Compact"}),
            C("appearance", "accent-mode", "Accent Mode", 0, {"Per Window", "Custom Hue", "Rainbow"}),
            S("appearance", "accent-hue", "Accent Hue", 210, 0, 360, 10),
            S("appearance", "rainbow-speed", "Rainbow Speed", 30, 5, 180, 5),
            S("appearance", "panel-opacity", "Panel Opacity", 75, 30, 100, 5),
            S("appearance", "title-opacity", "Title Opacity", 90, 30, 100, 5),
            S("appearance", "dim", "Background Dim", 45, 0, 90, 5),
            S("appearance", "ui-scale", "UI Scale", 1.0f, 0.6f, 1.4f, 0.05f),
            S("appearance", "win-width", "Window Width", 168, 120, 240, 8),
            S("appearance", "row-height", "Row Height", 24, 18, 36, 2),
            S("appearance", "title-height", "Title Height", 20, 16, 32, 2),
            S("appearance", "rows-per-page", "Rows Per Page", 5, 3, 8, 1),
            S("appearance", "label-scale", "Label Size", 0.38f, 0.25f, 0.6f, 0.02f),
            S("appearance", "check-scale", "Checkbox Size", 1.0f, 0.6f, 1.4f, 0.1f),
            C("appearance", "text-color", "Text Color", 0, {"White", "Yellow", "Cyan", "Green"}),
            C("appearance", "menu-font", "Menu Font", 0, {"Big", "Gold"}),
            T("appearance", "high-contrast", "High Contrast"),
            T("appearance", "row-stripes", "Row Stripes", true),
            T("appearance", "row-highlight", "Press Highlight", true),
            T("appearance", "accent-underline", "Accent Edge Line", true),
            T("appearance", "show-page-num", "Show Page Number", true),
            B("appearance", "reset-theme", "Reset Appearance"),
        }},
        {"behavior", "Behavior", {80, 170, 255}, {
            T("behavior", "reduce-motion", "Reduce Motion"),
            T("behavior", "anim-open", "Open Animation", true),
            S("behavior", "anim-speed", "Open Anim Time", 0.25f, 0.1f, 0.8f, 0.05f),
            S("behavior", "open-btn-scale", "Open Button Size", 0.6f, 0.4f, 1.2f, 0.1f),
            T("behavior", "back-closes", "Back Closes Menu", true),
            T("behavior", "remember-pos", "Remember Positions", true),
            T("behavior", "remember-collapsed", "Remember Collapsed", true),
            T("behavior", "snap-grid", "Snap To Grid"),
            C("behavior", "grid-size", "Grid Size", 1, {"8", "16", "32"}),
            T("behavior", "snap-edges", "Snap To Edges"),
            S("behavior", "snap-dist", "Snap Distance", 12, 4, 40, 2),
            T("behavior", "front-on-touch", "Raise On Touch", true),
            T("behavior", "lock-windows", "Lock Windows"),
            S("behavior", "safe-margin", "Safe Margin", 10, 0, 40, 2),
            T("behavior", "accordion", "Accordion Mode"),
            T("behavior", "click-sound", "Click Sound"),
            T("behavior", "start-collapsed", "Start Collapsed", true),
            T("behavior", "notify", "Notifications", true),
            T("behavior", "tap-title", "Tap Title Collapses", true),
            T("behavior", "page-wrap", "Pager Wraps Around", true),
            T("behavior", "confirm-reset", "Confirm Reset All", true),
            S("behavior", "arrange-gap", "Arrange Gap", 6, 2, 20, 2),
            B("behavior", "reset-windows", "Reset Window Positions"),
            B("behavior", "arrange", "Auto Arrange"),
            B("behavior", "collapse-all", "Collapse All"),
            B("behavior", "expand-all", "Expand All"),
            B("behavior", "reset-all", "Reset All Settings"),
        }},
        {"shortcuts", "Shortcuts", {120, 220, 120}, {
            B("shortcuts", "sc-overlay", "Toggle Overlay"),
            B("shortcuts", "sc-stats", "Toggle Cocos Stats"),
            B("shortcuts", "sc-fps-cycle", "Cycle FPS Preset"),
            B("shortcuts", "sc-saver", "Toggle Battery Saver"),
            B("shortcuts", "sc-graph", "Toggle FPS Graph"),
            B("shortcuts", "sc-corner", "Cycle Overlay Corner"),
            B("shortcuts", "sc-theme", "Cycle Layout Preset"),
            B("shortcuts", "sc-contrast", "Toggle High Contrast"),
            B("shortcuts", "sc-close", "Close Menu"),
        }},
        {"info", "Info", {220, 200, 80}, {
            I("info", "info-mod", "Mod Version"),
            I("info", "info-geode", "Geode Version"),
            I("info", "info-gd", "GD Version"),
            I("info", "info-arch", "Architecture"),
            I("info", "info-res", "Resolution"),
            I("info", "info-target", "FPS Target"),
            I("info", "info-session", "Session"),
            I("info", "info-frames", "Frames"),
            I("info", "info-counts", "Features"),
            B("info", "info-copy", "Copy Debug Info"),
        }},
    };
    return defs;
}

inline Feature const* feat(std::string const& id) {
    static std::unordered_map<std::string, Feature const*> idx = [] {
        std::unordered_map<std::string, Feature const*> m;
        for (auto const& w : windowDefs())
            for (auto const& f : w.features) m[f.id] = &f;
        return m;
    }();
    auto it = idx.find(id);
    return it == idx.end() ? nullptr : it->second;
}

inline int interactiveFeatureCount() {
    int n = 0;
    for (auto const& w : windowDefs())
        for (auto const& f : w.features)
            if (f.kind != Kind::Info) ++n;
    return n;
}

// ---- persistence (keys keep the original "opt-" prefix) --------------------
inline std::string key(std::string const& id) { return "opt-" + id; }

inline bool bval(std::string const& id) {
    auto f = feat(id);
    return Mod::get()->getSavedValue<bool>(key(id), f ? f->def != 0.f : false);
}
inline float fval(std::string const& id) {
    auto f = feat(id);
    return Mod::get()->getSavedValue<float>(key(id), f ? f->def : 0.f);
}
inline int ival(std::string const& id) {
    auto f = feat(id);
    return Mod::get()->getSavedValue<int>(key(id), f ? (int)f->def : 0);
}
inline void setB(std::string const& id, bool v) { Mod::get()->setSavedValue<bool>(key(id), v); }
inline void setF(std::string const& id, float v) { Mod::get()->setSavedValue<float>(key(id), v); }
inline void setI(std::string const& id, int v) { Mod::get()->setSavedValue<int>(key(id), v); }

// Writes a raw value using the feature's own kind (used by presets and resets).
inline void setRaw(std::string const& id, float v) {
    auto f = feat(id);
    if (!f) return;
    if (f->kind == Kind::Toggle) setB(id, v != 0.f);
    else if (f->kind == Kind::Choice) setI(id, (int)std::lround(v));
    else if (f->kind == Kind::Slider) setF(id, v);
}

// ---- small helpers ---------------------------------------------------------
inline GLubyte pctByte(float pct) { return (GLubyte)(std::clamp(pct, 0.f, 100.f) * 2.55f); }

inline ccColor3B hsv(float h, float s, float v) {
    h = std::fmod(std::fmod(h, 360.f) + 360.f, 360.f) / 60.f;
    int i = (int)h;
    float f = h - i, p = v * (1 - s), q = v * (1 - s * f), t = v * (1 - s * (1 - f));
    float r = v, g = t, b = p;
    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
    return {(GLubyte)(r * 255), (GLubyte)(g * 255), (GLubyte)(b * 255)};
}

inline ccColor3B accentFor(WindowDef const& def, float rainbowHue) {
    switch (ival("accent-mode")) {
        case 1: return hsv(fval("accent-hue"), 0.55f, 0.9f);
        case 2: return hsv(rainbowHue + (float)(&def - &windowDefs()[0]) * 40.f, 0.55f, 0.9f);
        default: return def.accent;
    }
}

inline ccColor3B menuTextColor() {
    switch (ival("text-color")) {
        case 1: return {255, 240, 120};
        case 2: return {130, 240, 255};
        case 3: return {140, 255, 150};
        default: return {255, 255, 255};
    }
}

inline const char* menuFont() { return ival("menu-font") == 1 ? "goldFont.fnt" : "bigFont.fnt"; }

inline void toast(std::string const& msg, NotificationIcon icon = NotificationIcon::None) {
    if (bval("notify")) Notification::create(msg, icon, 1.5f)->show();
}

} // namespace mm
