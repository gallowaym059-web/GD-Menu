#pragma once
#include "Actions.hpp"

namespace mm {

// One draggable category window. Rows are drawn as plain nodes and hit-tested by
// the owning layer (no CCMenu), so touch routing is fully deterministic.
class MenuWindow : public CCLayerColor {
public:
    enum Zone { ZNone, ZRow, ZMinus, ZPlus, ZPrev, ZNext, ZCollapse, ZTitle };
    struct Hit {
        Zone zone = ZNone;
        int row = -1;
        bool operator==(Hit const& o) const { return zone == o.zone && row == o.row; }
    };
    using Callback = std::function<void(MenuWindow*, unsigned)>;

protected:
    static constexpr float kPad = 6.f;
    static constexpr float kBottomPad = 4.f;

    struct Row {
        Feature const* f = nullptr;
        CCLayerColor* bg = nullptr;
        CCLabelBMFont* value = nullptr;
        CCSprite* check = nullptr;
        GLubyte baseOp = 0;
        float cy = 0.f;
        CCRect rect, minus, plus;
    };

    WindowDef const* m_def = nullptr;
    std::vector<Row> m_rows;
    CCNode* m_body = nullptr;
    CCLayerColor* m_titleBar = nullptr;
    CCLayerColor* m_edge = nullptr;
    Callback m_cb;
    ccColor3B m_accent = {255, 255, 255};
    bool m_collapsed = false;
    bool m_built = false;
    int m_page = 0, m_pages = 1, m_perPage = 5;
    float m_w = 168.f, m_rowH = 24.f, m_titleH = 20.f, m_fullH = 100.f;

    bool init(WindowDef const& def) {
        if (!CCLayerColor::initWithColor({15, 15, 20, 190})) return false;
        m_def = &def;
        this->setAnchorPoint({0.f, 0.f});
        this->ignoreAnchorPointForPosition(false);
        int saved = Mod::get()->getSavedValue<int>("collapsed-" + std::string(def.id), -1);
        if (bval("remember-collapsed") && saved >= 0) m_collapsed = saved != 0;
        else m_collapsed = bval("start-collapsed") && std::string(def.id) != "display";
        this->build();
        return true;
    }

    CCLabelBMFont* makeLabel(const char* text, float maxW) {
        auto l = CCLabelBMFont::create(text, menuFont());
        l->limitLabelWidth(maxW, fval("label-scale"), 0.1f);
        l->setColor(menuTextColor());
        return l;
    }

    void makeCheck(Row& r) {
        if (r.check) r.check->removeFromParent();
        r.check = CCSprite::createWithSpriteFrameName(bval(r.f->id) ? "GJ_checkOn_001.png" : "GJ_checkOff_001.png");
        if (!r.check) return; // sprite sheet missing: row stays usable, just without a box
        float s = (m_rowH * 0.78f / r.check->getContentSize().height) * fval("check-scale");
        r.check->setScale(s);
        r.check->setPosition({m_w - kPad - m_rowH * 0.45f, r.cy});
        m_body->addChild(r.check, 2);
    }

    void addRow(int i, Feature const& f) {
        float y0 = m_fullH - m_titleH - (i + 1) * m_rowH;
        Row r;
        r.f = &f;
        r.cy = y0 + m_rowH / 2;
        r.rect = CCRect(0, y0, m_w, m_rowH);
        bool button = f.kind == Kind::Button;
        r.baseOp = button ? 55 : ((bval("row-stripes") && (i % 2 == 1)) ? 22 : 0);
        r.bg = CCLayerColor::create({255, 255, 255, r.baseOp}, m_w, m_rowH - 1.f);
        r.bg->setColor(button ? m_accent : ccColor3B{255, 255, 255});
        r.bg->setPosition({0.f, y0});
        m_body->addChild(r.bg, 0);

        switch (f.kind) {
            case Kind::Toggle: {
                auto l = makeLabel(f.label, m_w - m_rowH - 2 * kPad);
                l->setAnchorPoint({0.f, 0.5f});
                l->setPosition({kPad, r.cy});
                m_body->addChild(l, 1);
                makeCheck(r);
                break;
            }
            case Kind::Slider: {
                float btnW = m_rowH * 1.1f, valW = 40.f, cluster = valW + 2 * btnW;
                float x0 = m_w - kPad - cluster;
                auto l = makeLabel(f.label, x0 - 2 * kPad);
                l->setAnchorPoint({0.f, 0.5f});
                l->setPosition({kPad, r.cy});
                m_body->addChild(l, 1);
                r.minus = CCRect(x0, y0, btnW, m_rowH);
                r.plus = CCRect(x0 + btnW + valW, y0, btnW, m_rowH);
                for (int k = 0; k < 2; ++k) {
                    float bx = k == 0 ? x0 : x0 + btnW + valW;
                    auto box = CCLayerColor::create({255, 255, 255, 40}, btnW - 2.f, m_rowH - 7.f);
                    box->setPosition({bx + 1.f, y0 + 3.f});
                    m_body->addChild(box, 1);
                    auto g = CCLabelBMFont::create(k == 0 ? "-" : "+", "bigFont.fnt");
                    g->setScale(0.55f);
                    g->setPosition({bx + btnW / 2, r.cy});
                    m_body->addChild(g, 2);
                }
                r.value = CCLabelBMFont::create(valueText(f).c_str(), menuFont());
                r.value->limitLabelWidth(valW - 2.f, 0.4f, 0.1f);
                r.value->setColor(m_accent);
                r.value->setPosition({x0 + btnW + valW / 2, r.cy});
                m_body->addChild(r.value, 2);
                break;
            }
            case Kind::Choice:
            case Kind::Info: {
                float vw = (m_w - 3 * kPad) * (f.kind == Kind::Info ? 0.5f : 0.42f);
                auto l = makeLabel(f.label, m_w - vw - 3 * kPad);
                l->setAnchorPoint({0.f, 0.5f});
                l->setPosition({kPad, r.cy});
                m_body->addChild(l, 1);
                r.value = CCLabelBMFont::create(valueText(f).c_str(), menuFont());
                r.value->limitLabelWidth(vw, 0.4f, 0.1f);
                r.value->setColor(m_accent);
                r.value->setAnchorPoint({1.f, 0.5f});
                r.value->setPosition({m_w - kPad, r.cy});
                m_body->addChild(r.value, 2);
                break;
            }
            case Kind::Button: {
                auto l = makeLabel(f.label, m_w - 2 * kPad);
                l->setPosition({m_w / 2, r.cy});
                m_body->addChild(l, 1);
                break;
            }
        }
        m_rows.push_back(r);
    }

    void build() {
        this->removeAllChildrenWithCleanup(true);
        m_rows.clear();
        m_body = nullptr; m_edge = nullptr; m_titleBar = nullptr;

        m_w = fval("win-width");
        m_rowH = fval("row-height");
        m_titleH = fval("title-height");
        m_perPage = std::max(1, (int)std::lround(fval("rows-per-page")));
        int n = (int)m_def->features.size();
        m_pages = std::max(1, (n + m_perPage - 1) / m_perPage);
        m_page = std::clamp(m_page, 0, m_pages - 1);
        int vis = std::min(n, m_perPage);
        float footer = m_pages > 1 ? m_rowH : 0.f;
        m_fullH = m_titleH + vis * m_rowH + footer + kBottomPad;
        float h = m_collapsed ? m_titleH : m_fullH;
        this->setContentSize({m_w, h});

        bool hc = bval("high-contrast");
        this->setColor(hc ? ccColor3B{0, 0, 0} : ccColor3B{15, 15, 20});
        this->setOpacity(hc ? 245 : pctByte(fval("panel-opacity")));
        m_accent = accentFor(*m_def, g_rainbowHue);

        m_titleBar = CCLayerColor::create({m_accent.r, m_accent.g, m_accent.b, pctByte(fval("title-opacity"))}, m_w, m_titleH);
        m_titleBar->setPosition({0.f, h - m_titleH});
        this->addChild(m_titleBar, 2);
        auto title = CCLabelBMFont::create(m_def->title, menuFont());
        title->limitLabelWidth(m_w - 2.f * m_titleH, 0.45f, 0.1f);
        title->setAnchorPoint({0.f, 0.5f});
        title->setPosition({kPad, m_titleH / 2});
        m_titleBar->addChild(title);
        auto glyph = CCLabelBMFont::create(m_collapsed ? "+" : "-", "bigFont.fnt");
        glyph->setScale(0.6f);
        glyph->setPosition({m_w - m_titleH * 0.65f, m_titleH / 2});
        m_titleBar->addChild(glyph);

        if (bval("accent-underline")) {
            m_edge = CCLayerColor::create({m_accent.r, m_accent.g, m_accent.b, 255}, m_w, 1.5f);
            this->addChild(m_edge, 2);
        }

        if (!m_collapsed) {
            m_body = CCNode::create();
            this->addChild(m_body, 1);
            int first = m_page * m_perPage;
            for (int i = 0; i < vis && first + i < n; ++i) addRow(i, m_def->features[first + i]);
            if (m_pages > 1) {
                for (int k = 0; k < 2; ++k) {
                    auto box = CCLayerColor::create({255, 255, 255, 40}, m_w / 2 - 6.f, m_rowH - 7.f);
                    box->setPosition({k == 0 ? 4.f : m_w / 2 + 2.f, kBottomPad + 3.f});
                    m_body->addChild(box, 1);
                    auto g = CCLabelBMFont::create(k == 0 ? "<" : ">", "bigFont.fnt");
                    g->setScale(0.55f);
                    g->setPosition({k == 0 ? m_w / 4 : m_w * 0.75f, kBottomPad + m_rowH / 2});
                    m_body->addChild(g, 2);
                }
                if (bval("show-page-num")) {
                    char b[24];
                    std::snprintf(b, sizeof b, "%d/%d", m_page + 1, m_pages);
                    auto pl = CCLabelBMFont::create(b, "bigFont.fnt");
                    pl->setScale(0.35f);
                    pl->setColor(m_accent);
                    pl->setPosition({m_w / 2, kBottomPad + m_rowH / 2});
                    m_body->addChild(pl, 3);
                }
            }
        }
        m_built = true;
    }

    void changePage(int delta) {
        int p = m_page + delta;
        if (bval("page-wrap")) p = (p + m_pages) % m_pages;
        p = std::clamp(p, 0, m_pages - 1);
        if (p == m_page) return;
        m_page = p;
        this->rebuild();
    }

public:
    static MenuWindow* create(WindowDef const& def) {
        auto r = new MenuWindow();
        if (r && r->init(def)) { r->autorelease(); return r; }
        CC_SAFE_DELETE(r);
        return nullptr;
    }

    void setCallback(Callback cb) { m_cb = std::move(cb); }
    WindowDef const& def() const { return *m_def; }
    bool collapsed() const { return m_collapsed; }

    // Rebuilds contents but keeps the window's top edge where it was.
    void rebuild() {
        float top = m_built ? this->getPositionY() + this->getContentSize().height * this->getScale() : 0.f;
        this->build();
        if (m_built && top != 0.f) this->setPositionY(top - this->getContentSize().height * this->getScale());
    }

    void setCollapsed(bool c) {
        if (c == m_collapsed) return;
        m_collapsed = c;
        if (bval("remember-collapsed"))
            Mod::get()->setSavedValue<int>("collapsed-" + std::string(m_def->id), c ? 1 : 0);
        this->rebuild();
    }

    void setAccent(ccColor3B c) {
        m_accent = c;
        if (m_titleBar) m_titleBar->setColor(c);
        if (m_edge) m_edge->setColor(c);
    }

    CCSize scaledSize() { return {this->getContentSize().width * this->getScale(), this->getContentSize().height * this->getScale()}; }

    bool contains(CCPoint const& wp) {
        auto p = this->convertToNodeSpace(wp);
        auto s = this->getContentSize();
        return CCRect(0, 0, s.width, s.height).containsPoint(p);
    }

    Hit hitTest(CCPoint const& wp) {
        Hit h;
        auto p = this->convertToNodeSpace(wp);
        float ch = this->getContentSize().height;
        if (!CCRect(0, 0, m_w, ch).containsPoint(p)) return h;
        if (p.y >= ch - m_titleH) {
            h.zone = p.x >= m_w - m_titleH * 1.4f ? ZCollapse : ZTitle;
            return h;
        }
        if (m_collapsed) return h;
        for (int i = 0; i < (int)m_rows.size(); ++i) {
            auto const& r = m_rows[i];
            if (!r.rect.containsPoint(p)) continue;
            h.row = i;
            if (r.f->kind == Kind::Slider) {
                if (r.minus.containsPoint(p)) h.zone = ZMinus;
                else if (r.plus.containsPoint(p)) h.zone = ZPlus;
                else h.row = -1;
            } else h.zone = ZRow;
            return h;
        }
        if (m_pages > 1 && p.y < m_rowH + kBottomPad) h.zone = p.x < m_w / 2 ? ZPrev : ZNext;
        return h;
    }

    void setPressed(Hit const& h, bool on) {
        if (!bval("row-highlight")) return;
        if (h.row < 0 || h.row >= (int)m_rows.size()) return;
        auto const& r = m_rows[h.row];
        r.bg->setColor(on ? m_accent : (r.f->kind == Kind::Button ? m_accent : ccColor3B{255, 255, 255}));
        r.bg->setOpacity(on ? 95 : r.baseOp);
    }

    void refreshValues() {
        for (auto& r : m_rows) {
            if (r.f->kind == Kind::Toggle) makeCheck(r);
            else if (r.value) r.value->setString(valueText(*r.f).c_str());
        }
    }

    // May rebuild this window: callers must not touch row state afterwards.
    void activate(Hit const& h) {
        switch (h.zone) {
            case ZCollapse:
            case ZTitle:
                this->setCollapsed(!m_collapsed);
                if (m_cb) m_cb(this, FxToggled);
                break;
            case ZPrev: changePage(-1); break;
            case ZNext: changePage(+1); break;
            case ZRow:
            case ZMinus:
            case ZPlus:
                if (h.row >= 0 && h.row < (int)m_rows.size()) {
                    auto const* f = m_rows[h.row].f; // registry data outlives rows
                    unsigned fx = perform(*f, h.zone == ZMinus ? -1 : +1);
                    if (m_cb) m_cb(this, fx);
                }
                break;
            default: break;
        }
    }
};

} // namespace mm
