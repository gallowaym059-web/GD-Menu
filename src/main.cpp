// Mobile Window Menu: touch-first, draggable category windows for Geode (Android32 + Android64).
// File map: Settings.hpp (feature registry/persistence), Fps.hpp, StatsOverlay.hpp,
// Actions.hpp (what each control does), MenuWindow.hpp, ModMenuLayer.hpp.
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include "ModMenuLayer.hpp"

using namespace geode::prelude;

class $modify(MobileMenuHook, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        // Restore saved display state at startup.
        if (mm::bval("fps-startup")) mm::applyFps();
        else if (auto d = CCDirector::get()) d->setDisplayStats(mm::bval("show-fps"));
        mm::StatsOverlay::ensure();

        auto spr = ButtonSprite::create("Menu", "bigFont.fnt", "GJ_button_01.png", 0.6f);
        auto btn = CCMenuItemSpriteExtra::create(
            spr, this, menu_selector(MobileMenuHook::onOpenModMenu));
        btn->setID("open-mobile-menu"_spr);
        btn->setScale(mm::fval("open-btn-scale") / 0.6f);

        if (auto bottom = this->getChildByID("bottom-menu")) {
            bottom->addChild(btn);
            bottom->updateLayout();
        }
        return true;
    }

    void onOpenModMenu(CCObject*) { mm::ModMenuLayer::open(); }
};
