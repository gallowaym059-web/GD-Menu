#pragma once
#include "MenuWindow.hpp"

namespace mm {

class ModMenuLayer : public CCLayer {
protected:
    static constexpr int kLayerPrio = -500;      // above CCMenu (-128): the menu eats all touches
    static constexpr float kTapSlop = 8.f;       // movement below this still counts as a tap
    static constexpr float kCloseSize = 34.f;

    std::vector<MenuWindow*> m_windows;
    CCLayerColor* m_dim = nullptr;
    CCSprite* m_closeSpr = nullptr;
    CCRect m_closeRect;
    CCSize m_lastWin;

    // Single-touch state machine.
    CCTouch* m_touch = nullptr;
    MenuWindow* m_hitWin = nullptr;
    MenuWindow::Hit m_hit;
    CCPoint m_start, m_dragOffset;
    bool m_dragging = false, m_cancelled = false, m_closePressed = false;
    bool m_rainbow = false;
    float m_uiScale = 1.f;

    bool init() override {
        if (!CCLayer::init()) return false;
        this->setTouchEnabled(true);
        this->setKeypadEnabled(true);
        this->setTouchPriority(kLayerPrio);
        this->setTouchMode(kCCTouchesOneByOne);

        auto win = CCDirector::get()->getWinSize();
        m_lastWin = win;
        m_dim = CCLayerColor::create({0, 0, 0, pctByte(fval("dim"))}, win.width, win.height);
        this->addChild(m_dim, 0);

        for (auto const& def : windowDefs()) {
            auto w = MenuWindow::create(def);
            if (!w) continue;
            w->setCallback([this](MenuWindow* src, unsigned fx) { this->onFx(src, fx); });
            this->addChild(w, 1 + (int)m_windows.size());
            m_windows.push_back(w);
        }
        this->applyScale();
        this->arrange();
        this->restoreSavedPositions();

        m_closeSpr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        if (m_closeSpr) {
            m_closeSpr->setScale(0.7f);
            this->addChild(m_closeSpr, 100);
        }
        this->layoutClose();
        this->clampAll();
        this->playOpenAnimation();
        this->scheduleUpdate(); // only runs while the menu exists
        return true;
    }

    void onEnter() override { CCLayer::onEnter(); g_menuOpen = true; StatsOverlay::reloadAll(); }
    void onExit() override { g_menuOpen = false; StatsOverlay::reloadAll(); CCLayer::onExit(); }

    void layoutClose() {
        auto win = CCDirector::get()->getWinSize();
        float m = fval("safe-margin");
        CCPoint c(win.width - m - kCloseSize / 2, win.height - m - kCloseSize / 2);
        if (m_closeSpr) m_closeSpr->setPosition(c);
        m_closeRect = CCRect(c.x - kCloseSize / 2, c.y - kCloseSize / 2, kCloseSize, kCloseSize);
    }

    void applyScale() {
        m_uiScale = fval("ui-scale");
        for (auto w : m_windows) w->setScale(m_uiScale);
    }

    void playOpenAnimation() {
        if (bval("reduce-motion") || !bval("anim-open")) return;
        float dur = fval("anim-speed");
        m_dim->setOpacity(0);
        m_dim->runAction(CCFadeTo::create(dur, pctByte(fval("dim"))));
        for (auto w : m_windows) {
            w->setScale(m_uiScale * 0.85f);
            w->runAction(CCEaseBackOut::create(CCScaleTo::create(dur, m_uiScale)));
        }
    }

    // Deterministic, non-overlapping column flow from the top-left safe corner.
    void arrange() {
        auto win = CCDirector::get()->getWinSize();
        float m = fval("safe-margin"), gap = fval("arrange-gap");
        float x = m, top = win.height - m, y = top, colW = 0.f;
        for (auto w : m_windows) {
            auto s = w->scaledSize();
            if (y != top && y - s.height < m) { x += colW + gap; y = top; colW = 0.f; }
            w->setPosition({x, y - s.height});
            y -= s.height + gap;
            colW = std::max(colW, s.width);
        }
        this->clampAll();
    }

    void restoreSavedPositions() {
        if (!bval("remember-pos")) return;
        for (auto w : m_windows) {
            std::string id = w->def().id;
            float sx = store::get<float>("x-" + id, -9999.f);
            float sy = store::get<float>("y-" + id, -9999.f);
            if (sx > -9000.f && sy > -9000.f) w->setPosition({sx, sy});
        }
    }

    void clearSavedPositions() {
        for (auto w : m_windows) {
            std::string id = w->def().id;
            store::set<float>("x-" + id, -9999.f);
            store::set<float>("y-" + id, -9999.f);
        }
    }

    void clampToScreen(MenuWindow* w) {
        auto win = CCDirector::get()->getWinSize();
        auto s = w->scaledSize();
        float m = fval("safe-margin");
        float hiX = std::max(m, win.width - s.width - m);
        float hiY = std::max(m, win.height - s.height - m);
        w->setPosition({std::clamp(w->getPositionX(), m, hiX), std::clamp(w->getPositionY(), m, hiY)});
    }
    void clampAll() { for (auto w : m_windows) clampToScreen(w); }

    void savePosition(MenuWindow* w) {
        if (!bval("remember-pos")) return;
        std::string id = w->def().id;
        store::set<float>("x-" + id, w->getPositionX());
        store::set<float>("y-" + id, w->getPositionY());
    }

    void snap(MenuWindow* w) {
        auto win = CCDirector::get()->getWinSize();
        auto s = w->scaledSize();
        float m = fval("safe-margin");
        float x = w->getPositionX(), y = w->getPositionY();
        if (bval("snap-grid")) {
            static constexpr float kGrid[3] = {8.f, 16.f, 32.f};
            float g = kGrid[std::clamp(ival("grid-size"), 0, 2)];
            x = std::round(x / g) * g;
            y = std::round(y / g) * g;
        }
        if (bval("snap-edges")) {
            float d = fval("snap-dist");
            if (std::fabs(x - m) < d) x = m;
            if (std::fabs(x + s.width - (win.width - m)) < d) x = win.width - m - s.width;
            if (std::fabs(y - m) < d) y = m;
            if (std::fabs(y + s.height - (win.height - m)) < d) y = win.height - m - s.height;
        }
        w->setPosition({x, y});
        clampToScreen(w);
    }

    void bringToFront(MenuWindow* w) {
        int top = 0;
        for (auto o : m_windows) top = std::max(top, o->getZOrder());
        this->reorderChild(w, top + 1);
    }

    void applyTheme() {
        m_dim->setOpacity(pctByte(fval("dim")));
        this->applyScale();
        m_rainbow = ival("accent-mode") == 2 && !bval("reduce-motion");
        for (auto w : m_windows) w->rebuild();
        this->layoutClose();
        this->clampAll();
    }

    void onFx(MenuWindow* src, unsigned fx) {
        if (fx & FxResetPos) { clearSavedPositions(); this->arrange(); }
        if (fx & FxArrange) this->arrange();
        if (fx & FxCollapse) for (auto w : m_windows) w->setCollapsed(true);
        if (fx & FxExpand) for (auto w : m_windows) w->setCollapsed(false);
        if (fx & FxRebuild) this->applyTheme();
        if (fx & FxValues) for (auto w : m_windows) w->refreshValues();
        if ((fx & FxToggled) && src && !src->collapsed() && bval("accordion"))
            for (auto w : m_windows) if (w != src) w->setCollapsed(true);
        if (fx & (FxToggled | FxCollapse | FxExpand)) this->clampAll();
        if (fx & FxClose) this->onClose();
    }

    void clearTouch() {
        if (m_hitWin) m_hitWin->setPressed(m_hit, false);
        m_touch = nullptr; m_hitWin = nullptr; m_hit = {};
        m_dragging = false; m_cancelled = false; m_closePressed = false;
    }

    bool ccTouchBegan(CCTouch* t, CCEvent*) override {
        if (m_touch) return true; // extra fingers are swallowed, never forwarded to the game
        auto p = t->getLocation();
        m_touch = t; m_start = p;
        m_dragging = false; m_cancelled = false; m_hitWin = nullptr; m_hit = {};
        m_closePressed = m_closeRect.containsPoint(p);
        if (m_closePressed) return true;

        MenuWindow* top = nullptr;
        for (auto w : m_windows)
            if (w->contains(p) && (!top || w->getZOrder() > top->getZOrder())) top = w;
        if (top) {
            if (bval("front-on-touch")) this->bringToFront(top);
            m_hitWin = top;
            m_hit = top->hitTest(p);
            top->setPressed(m_hit, true);
            if (m_hit.zone == MenuWindow::ZTitle && !bval("lock-windows"))
                m_dragOffset = top->getPosition() - p;
        }
        return true; // swallow everything: nothing leaks to the game below
    }

    void ccTouchMoved(CCTouch* t, CCEvent*) override {
        if (t != m_touch) return;
        auto p = t->getLocation();
        if (!m_dragging && !m_cancelled && ccpDistance(p, m_start) > kTapSlop) {
            if (m_hitWin && m_hit.zone == MenuWindow::ZTitle && !bval("lock-windows")) m_dragging = true;
            else m_cancelled = true; // finger slid off a row: not a tap anymore
            if (m_hitWin) m_hitWin->setPressed(m_hit, false);
        }
        if (m_dragging && m_hitWin) {
            m_hitWin->setPosition(p + m_dragOffset);
            this->clampToScreen(m_hitWin);
        }
    }

    void ccTouchEnded(CCTouch* t, CCEvent*) override {
        if (t != m_touch) return;
        auto p = t->getLocation();
        auto win = m_hitWin;
        auto hit = m_hit;
        bool closeTap = m_closePressed && m_closeRect.containsPoint(p);
        bool dragged = m_dragging, cancelled = m_cancelled;
        if (dragged && win) { this->snap(win); this->savePosition(win); }
        this->clearTouch(); // un-highlight first: activation may rebuild the window
        if (closeTap) { this->onClose(); return; }
        if (!win || dragged || cancelled) return;
        if (win->hitTest(p) == hit) {
            if (hit.zone == MenuWindow::ZTitle && !bval("tap-title")) return;
            win->activate(hit);
        }
    }

    void ccTouchCancelled(CCTouch* t, CCEvent*) override {
        if (t != m_touch) return;
        if (m_dragging && m_hitWin) this->savePosition(m_hitWin);
        this->clearTouch();
    }

    void update(float dt) override {
        auto ws = CCDirector::get()->getWinSize();
        if (ws.width != m_lastWin.width || ws.height != m_lastWin.height) { // orientation / resize
            m_lastWin = ws;
            m_dim->setContentSize(ws);
            this->layoutClose();
            this->clampAll();
        }
        if (m_rainbow) {
            g_rainbowHue = std::fmod(g_rainbowHue + dt * fval("rainbow-speed"), 360.f);
            for (auto w : m_windows) w->setAccent(accentFor(w->def(), g_rainbowHue));
        }
    }

    void keyBackClicked() override {
        if (bval("back-closes")) this->onClose();
    }

public:
    static ModMenuLayer* create() {
        auto r = new ModMenuLayer();
        if (r && r->init()) { r->autorelease(); return r; }
        CC_SAFE_DELETE(r);
        return nullptr;
    }

    void onClose() {
        this->clearTouch();
        this->removeFromParentAndCleanup(true);
    }

    static void open() {
        auto scene = CCDirector::get()->getRunningScene();
        if (!scene || scene->getChildByID("mobile-mod-menu"_spr)) return;
        auto layer = ModMenuLayer::create();
        if (!layer) return;
        layer->m_rainbow = ival("accent-mode") == 2 && !bval("reduce-motion");
        layer->setID("mobile-mod-menu"_spr);
        scene->addChild(layer, 9999);
    }
};

} // namespace mm
