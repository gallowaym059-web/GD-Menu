#pragma once
#include "Settings.hpp"

namespace mm {

// Priority: Battery Saver (30) > Custom target > FPS Modifier (120) > default 60.
inline float targetFps() {
    if (bval("fps-saver")) return 30.f;
    if (bval("fps-custom")) return std::clamp(fval("fps-target"), 15.f, 360.f);
    return bval("fps-modifier") ? 120.f : 60.f;
}

// Requests an animation interval; the device/game/loader may still cap real FPS.
inline void applyFps() {
    auto d = CCDirector::get();
    if (!d) return;
    d->setAnimationInterval(1.0 / (double)targetFps());
    d->setDisplayStats(bval("show-fps"));
}

} // namespace mm
