// Mobile Window Menu: touch-first, draggable category windows for Geode (Android).
// File map: Settings.hpp (feature registry/persistence), Fps.hpp, StatsOverlay.hpp,
// Actions.hpp (what each control does), MenuWindow.hpp, ModMenuLayer.hpp, FloatButton.hpp.
//
// This mod installs NO hooks. A tiny pump re-queues itself on Geode's main-thread queue
// every frame and drives everything from cocos2d-x nodes, so it depends on neither
// Geometry Dash addresses nor on hooking any game or cocos function.
#include <Geode/Geode.hpp>
#include <chrono>

#include "FloatButton.hpp"

using namespace geode::prelude;

namespace {
    // Geode has exposed queueInMainThread both as a Loader method and as a free function.
    template <class F>
    void queueMain(F&& f) {
        if constexpr (requires { Loader::get()->queueInMainThread(std::forward<F>(f)); })
            Loader::get()->queueInMainThread(std::forward<F>(f));
        else
            queueInMainThread(std::forward<F>(f));
    }

    void pump() {
        using clock = std::chrono::steady_clock;
        static auto last = clock::now();
        auto now = clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;
        mm::frame(dt);
        queueMain([] { pump(); }); // runs again next frame
    }
}

$on_mod(Loaded) {
    queueMain([] { pump(); });
}
