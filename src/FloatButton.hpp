#pragma once
#include "ModMenuLayer.hpp"

namespace mm {

// Small draggable "M" button that opens the menu from any scene. Built only from
// cocos2d-x nodes (no Geometry Dash classes), so it does not depend on GD addresses.
class FloatButton : public CCLayer {
protected:
    static constexpr int kPrio = -400;   // below ModMenuLayer (-500), above game layers
    static constexpr float kBase = 34.f;
    static constexpr float kSlop = 8.f;
    inline static FloatButton* s_inst = nullptr;

    CCLayerColor* m_bg = nullptr;
    CCLabelBMFont* m_lbl = nullptr;
    CCTouch* m_touch = nullptr;
    CCPoint m_start, m_off;
    bool m_drag = false;
    float m_size = kBase;

    bool init() override {
        if (!CCLayer::init()) return false;
        this->setTouchEnabled(true);
        this->setTouchPriority(kPrio);
        this->setTouchMode(kCCTouchesOneByOne);
        m_bg = CCLayerColor::create({20, 20, 28, 200}, kBase, kBase);
        this->addChild(m_bg, 0);
        m_lbl = CCLabelBMFont::create("M", "bigFont.fnt");
        this->addChild(m_lbl, 1);
        this->reload(true);
        return true;
    }

    CCRect rect() { return CCRect(this->getPositionX(), this->getPositionY(), m_size, m_size); }

    void clampPos() {
        auto win = CCDirector::get()->getWinSize();
        float x = std::clamp(this->getPositionX(), 2.f, std::max(2.f, win.width - m_size - 2.f));
        float y = std::clamp(this->getPositionY(), 2.f, std::max(2.f, win.height - m_size - 2.f));
        this->setPosition({x, y});
    }

    bool ccTouchBegan(CCTouch* t, CCEvent*) override {
        if (m_touch || g_menuOpen || !this->isVisible()) return false;
        auto p = t->getLocation();
        if (!rect().containsPoint(p)) return false; // not ours: the game keeps the touch
        m_touch = t; m_start = p; m_drag = false;
        m_off = this->getPosition() - p;
        return true;
    }
    void ccTouchMoved(CCTouch* t, CCEvent*) override {
        if (t != m_touch) return;
        auto p = t->getLocation();
        if (!m_drag && ccpDistance(p, m_start) > kSlop) m_drag = true;
        if (m_drag) { this->setPosition(p + m_off); this->clampPos(); }
    }
    void ccTouchEnded(CCTouch* t, CCEvent*) override {
        if (t != m_touch) return;
        bool dragged = m_drag;
        m_touch = nullptr; m_drag = false;
        if (dragged) {
            store::set<float>("fx", this->getPositionX());
            store::set<float>("fy", this->getPositionY());
        } else ModMenuLayer::open();
    }
    void ccTouchCancelled(CCTouch* t, CCEvent*) override {
        if (t == m_touch) { m_touch = nullptr; m_drag = false; }
    }

public:
    static FloatButton* create() {
        auto r = new FloatButton();
        if (r && r->init()) { r->autorelease(); return r; }
        CC_SAFE_DELETE(r);
        return nullptr;
    }
    static FloatButton* get() { return s_inst; }
    static FloatButton* ensure() {
        if (!s_inst) {
            s_inst = create();
            if (s_inst) s_inst->retain(); // lives for the whole session
        }
        return s_inst;
    }

    void reload(bool first = false) {
        m_size = kBase * fval("open-btn-scale");
        m_bg->setContentSize({m_size, m_size});
        GLubyte op = pctByte(fval("open-btn-opacity"));
        m_bg->setOpacity((GLubyte)(op * 200 / 255));
        m_lbl->setOpacity(op);
        m_lbl->setScale(0.7f * fval("open-btn-scale"));
        m_lbl->setPosition({m_size / 2, m_size / 2});
        if (first) {
            auto win = CCDirector::get()->getWinSize();
            float sx = store::get<float>("fx", -9999.f);
            float sy = store::get<float>("fy", -9999.f);
            if (sx > -9000.f && sy > -9000.f) this->setPosition({sx, sy});
            else this->setPosition({6.f, win.height * 0.5f});
        }
        this->clampPos();
    }

    // Called every frame: keep attached to the running scene, hide while the menu is open.
    void tick() {
        auto scene = CCDirector::get()->getRunningScene();
        if (scene && this->getParent() != scene) {
            this->removeFromParentAndCleanup(false);
            scene->addChild(this, 100001);
        }
        this->setVisible(!g_menuOpen);
    }
};

inline void reloadFloatButton() {
    if (auto b = FloatButton::get()) b->reload();
}

// Per-frame entry point, called from the CCScheduler hook in main.cpp.
inline void frame(float dt) {
    static float waited = 0.f;
    static bool ready = false;
    auto d = CCDirector::get();
    if (!d || !d->getRunningScene()) return;
    if (!ready) {
        waited += dt;
        if (waited < 3.f) return; // let the game finish loading fonts/sprites first
        ready = true;
        if (bval("fps-startup")) applyFps();
        else d->setDisplayStats(bval("show-fps"));
        StatsOverlay::ensure();
        FloatButton::ensure();
    }
    if (auto o = StatsOverlay::get()) { o->attach(); o->step(dt); }
    if (auto b = FloatButton::get()) b->tick();
}

} // namespace mm
