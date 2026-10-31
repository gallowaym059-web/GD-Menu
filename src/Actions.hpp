#pragma once
#include "StatsOverlay.hpp"

namespace mm {

// Side effects a feature press asks the menu layer to perform.
enum Effect : unsigned {
    FxNone = 0, FxValues = 1, FxRebuild = 2, FxArrange = 4, FxClose = 8,
    FxCollapse = 16, FxExpand = 32, FxResetPos = 64, FxToggled = 128,
};

inline std::string sessionText() {
    auto o = StatsOverlay::get();
    int s = o ? (int)o->sessionSeconds() : 0;
    char b[32];
    std::snprintf(b, sizeof b, "%02d:%02d:%02d", s / 3600, (s / 60) % 60, s % 60);
    return b;
}

inline std::string infoText(std::string const& id) {
    char b[96];
    if (id == "info-mod") return Mod::get()->getVersion().toVString();
    if (id == "info-geode") return Loader::get()->getVersion().toVString();
    if (id == "info-gd") return Loader::get()->getGameVersion();
    if (id == "info-arch") return sizeof(void*) == 8 ? "64-bit" : "32-bit";
    if (id == "info-res") {
        auto w = CCDirector::get()->getWinSize();
        std::snprintf(b, sizeof b, "%.0fx%.0f", w.width, w.height);
        return b;
    }
    if (id == "info-target") { std::snprintf(b, sizeof b, "%.0f", targetFps()); return b; }
    if (id == "info-session") return sessionText();
    if (id == "info-frames") {
        auto o = StatsOverlay::get();
        std::snprintf(b, sizeof b, "%llu", o ? o->frames() : 0ull);
        return b;
    }
    if (id == "info-counts") { std::snprintf(b, sizeof b, "%d", interactiveFeatureCount()); return b; }
    return "";
}

inline std::string debugText() {
    std::string s = "Mobile Window Menu " + infoText("info-mod") + "\n";
    s += "Geode " + infoText("info-geode") + ", GD " + infoText("info-gd") + ", " + infoText("info-arch") + "\n";
    s += "Resolution " + infoText("info-res") + ", FPS target " + infoText("info-target") + "\n";
    s += "Session " + infoText("info-session") + ", frames " + infoText("info-frames") + "\n";
    return s;
}

inline float valueOf(Feature const& f) {
    if (f.kind == Kind::Toggle) return bval(f.id) ? 1.f : 0.f;
    if (f.kind == Kind::Choice) return (float)ival(f.id);
    return fval(f.id);
}

inline std::string valueText(Feature const& f) {
    char b[48];
    switch (f.kind) {
        case Kind::Slider: {
            float v = fval(f.id);
            if (f.step >= 1.f) std::snprintf(b, sizeof b, "%.0f", v);
            else std::snprintf(b, sizeof b, "%.2f", v);
            return b;
        }
        case Kind::Choice: {
            int i = std::clamp(ival(f.id), 0, (int)f.choices.size() - 1);
            return f.choices[i];
        }
        case Kind::Info: return infoText(f.id);
        default: return "";
    }
}

inline void applyPreset(int p) {
    struct V { const char* id; float v; };
    auto set = [](std::initializer_list<V> l) { for (auto const& x : l) setRaw(x.id, x.v); };
    switch (p) {
        case 1: // Megahack: dark, compact title bars, per-window accents
            set({{"panel-opacity", 75}, {"title-opacity", 90}, {"accent-mode", 0}, {"row-height", 22},
                 {"win-width", 160}, {"ui-scale", 1.f}, {"dim", 40}, {"high-contrast", 0}, {"text-color", 0}});
            break;
        case 2: // Eclipse: roomier rows, single blue accent
            set({{"panel-opacity", 85}, {"title-opacity", 95}, {"accent-mode", 1}, {"accent-hue", 210},
                 {"row-height", 26}, {"win-width", 176}, {"ui-scale", 1.f}, {"dim", 55}, {"text-color", 0}});
            break;
        case 3: // Compact: small windows for small screens
            set({{"panel-opacity", 70}, {"title-opacity", 85}, {"accent-mode", 0}, {"row-height", 20},
                 {"win-width", 140}, {"ui-scale", 0.8f}, {"dim", 20}, {"title-height", 18}});
            break;
        default: break;
    }
}

inline void resetAllSettings() {
    for (auto const& w : windowDefs()) {
        for (auto const& f : w.features) setRaw(f.id, f.def);
        std::string id = w.id;
        Mod::get()->setSavedValue<float>("x-" + id, -9999.f);
        Mod::get()->setSavedValue<float>("y-" + id, -9999.f);
        Mod::get()->setSavedValue<int>("collapsed-" + id, -1);
    }
}

inline unsigned runButton(std::string const& id) {
    static const std::pair<const char*, float> kPresets[] = {
        {"fps-p30", 30}, {"fps-p60", 60}, {"fps-p90", 90}, {"fps-p120", 120}, {"fps-p144", 144}, {"fps-p240", 240}};
    for (auto const& p : kPresets)
        if (id == p.first) {
            setF("fps-target", p.second); setB("fps-custom", true); setB("fps-saver", false);
            return FxValues;
        }
    static const float kCycle[] = {30, 60, 90, 120, 144, 240};

    if (id == "fps-reset") {
        setB("fps-custom", false); setB("fps-modifier", false); setB("fps-saver", false);
        setF("fps-target", 60.f);
    } else if (id == "ov-reset") StatsOverlay::resetAll();
    else if (id == "reset-windows") return FxResetPos | FxValues;
    else if (id == "arrange") return FxArrange;
    else if (id == "collapse-all") return FxCollapse;
    else if (id == "expand-all") return FxExpand;
    else if (id == "reset-theme") {
        for (auto const& w : windowDefs())
            if (std::string(w.id) == "appearance")
                for (auto const& f : w.features)
                    if (f.kind != Kind::Button) setRaw(f.id, f.def);
        return FxRebuild;
    } else if (id == "reset-all") {
        static std::time_t armedAt = 0;
        std::time_t now = std::time(nullptr);
        if (bval("confirm-reset") && (armedAt == 0 || now - armedAt > 4)) {
            armedAt = now;
            toast("Tap Reset All again to confirm", NotificationIcon::Warning);
            return FxNone;
        }
        armedAt = 0;
        resetAllSettings();
        applyFps();
        StatsOverlay::reloadAll();
        toast("All settings reset", NotificationIcon::Success);
        return FxRebuild | FxResetPos;
    } else if (id == "sc-overlay") setB("ov-enable", !bval("ov-enable"));
    else if (id == "sc-stats") setB("show-fps", !bval("show-fps"));
    else if (id == "sc-saver") setB("fps-saver", !bval("fps-saver"));
    else if (id == "sc-graph") setB("ov-graph", !bval("ov-graph"));
    else if (id == "sc-contrast") { setB("high-contrast", !bval("high-contrast")); return FxRebuild; }
    else if (id == "sc-corner") setI("ov-corner", (ival("ov-corner") + 1) % 4);
    else if (id == "sc-theme") {
        int p = (ival("layout-preset") + 1) % 4;
        setI("layout-preset", p);
        applyPreset(p);
        return FxRebuild;
    } else if (id == "sc-fps-cycle") {
        float cur = fval("fps-target");
        float next = kCycle[0];
        for (float c : kCycle) if (c > cur + 0.5f) { next = c; break; }
        setF("fps-target", next); setB("fps-custom", true); setB("fps-saver", false);
    } else if (id == "sc-close") return FxClose;
    else if (id == "info-copy") {
        utils::clipboard::write(debugText());
        toast("Debug info copied", NotificationIcon::Success);
    }
    return FxValues;
}

// Performs a press on a feature. dir = -1 for the minus half of a slider.
inline unsigned perform(Feature const& f, int dir) {
    unsigned fx = FxValues;
    std::string id = f.id;
    switch (f.kind) {
        case Kind::Toggle: setB(id, !bval(id)); break;
        case Kind::Slider: {
            float v = fval(id) + dir * f.step;
            v = std::round((v - f.min) / f.step) * f.step + f.min;
            setF(id, std::clamp(v, f.min, f.max));
            break;
        }
        case Kind::Choice: setI(id, (ival(id) + 1) % (int)f.choices.size()); break;
        case Kind::Button: fx |= runButton(id); break;
        default: return FxNone;
    }
    if (id == "layout-preset") applyPreset(ival(id));
    std::string w = f.window;
    // Only touch the director when an FPS-related control was pressed, so we never
    // override another mod's frame interval just because an unrelated row was tapped.
    if (w == "display" || w == "shortcuts" || id == "reset-all") applyFps();
    if (w == "overlay" || w == "shortcuts" || id == "reset-all") StatsOverlay::reloadAll();
    if (w == "appearance" || id == "open-btn-scale") fx |= FxRebuild;
    if (bval("click-sound")) FMODAudioEngine::sharedEngine()->playEffect("playSound_01.ogg");
    return fx;
}

} // namespace mm
