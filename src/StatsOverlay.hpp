#pragma once
#include "Fps.hpp"

// SceneManager is not pulled in by Geode.hpp; its header location differs between SDK versions.
#if __has_include(<Geode/ui/SceneManager.hpp>)
#include <Geode/ui/SceneManager.hpp>
#elif __has_include(<Geode/utils/SceneManager.hpp>)
#include <Geode/utils/SceneManager.hpp>
#endif

namespace mm {

inline int countNodes(CCNode* n) {
    if (!n) return 0;
    int c = 1;
    if (auto ch = n->getChildren())
        for (auto o : CCArrayExt<CCNode*>(ch)) c += countNodes(o);
    return c;
}

// Persistent (cross-scene) on-screen stats. No per-frame allocation: samples go
// into a fixed ring buffer; strings/graph are rebuilt only at the refresh rate.
class StatsOverlay : public CCNode {
protected:
    static constexpr int kMax = 240;
    inline static StatsOverlay* s_inst = nullptr;

    struct Cfg {
        bool enable = false, fps = true, ft = false, avg = false, mn = false, mx = false, low1 = false,
             jit = false, graph = false, clock = false, h24 = true, secs = true, date = false,
             session = false, frames = false, res = false, nodes = false, children = false,
             target = false, color = true, bg = true, compact = false, hideMenu = false;
        int corner = 0, rate = 1, decimals = 0, font = 0, samples = 120;
        float scale = 0.35f, opacity = 100, bgOpacity = 50, gh = 40, gw = 120, warn = 45, pad = 4;
    } m_c;

    CCLayerColor* m_bg = nullptr;
    CCLabelBMFont* m_label = nullptr;
    CCDrawNode* m_graph = nullptr;
    std::array<float, kMax> m_ft{};
    int m_head = 0, m_count = 0, m_nodes = 0, m_builtFont = -1;
    float m_ema = 0.f, m_acc = 0.f, m_nodeAcc = 99.f;
    double m_session = 0.0;
    unsigned long long m_frames = 0;

    bool init() override {
        if (!CCNode::init()) return false;
        m_bg = CCLayerColor::create({0, 0, 0, 120});
        this->addChild(m_bg, 0);
        m_graph = CCDrawNode::create();
        this->addChild(m_graph, 1);
        this->reload();
        this->scheduleUpdate();
        return true;
    }

    void rebuildLabel() {
        if (m_label) m_label->removeFromParent();
        m_label = CCLabelBMFont::create(" ", m_c.font == 1 ? "goldFont.fnt" : "bigFont.fnt");
        m_label->setAnchorPoint({0.f, 1.f});
        this->addChild(m_label, 2);
        m_builtFont = m_c.font;
    }

    void readCfg() {
        m_c.enable = bval("ov-enable"); m_c.fps = bval("ov-fps"); m_c.ft = bval("ov-frametime");
        m_c.avg = bval("ov-avg"); m_c.mn = bval("ov-min"); m_c.mx = bval("ov-max");
        m_c.low1 = bval("ov-low1"); m_c.jit = bval("ov-jitter"); m_c.graph = bval("ov-graph");
        m_c.clock = bval("ov-clock"); m_c.h24 = bval("ov-24h"); m_c.secs = bval("ov-seconds");
        m_c.date = bval("ov-date"); m_c.session = bval("ov-session"); m_c.frames = bval("ov-frames");
        m_c.res = bval("ov-res"); m_c.nodes = bval("ov-nodes"); m_c.children = bval("ov-children");
        m_c.target = bval("ov-target"); m_c.color = bval("ov-color"); m_c.bg = bval("ov-bg");
        m_c.compact = bval("ov-compact"); m_c.hideMenu = bval("ov-hide-menu");
        m_c.corner = std::clamp(ival("ov-corner"), 0, 3);
        m_c.rate = std::clamp(ival("ov-rate"), 0, 2);
        m_c.decimals = std::clamp(ival("ov-decimals"), 0, 2);
        m_c.font = std::clamp(ival("ov-font"), 0, 1);
        m_c.samples = std::clamp((int)fval("ov-samples"), 10, kMax);
        m_c.scale = fval("ov-scale"); m_c.opacity = fval("ov-opacity");
        m_c.bgOpacity = fval("ov-bgopacity"); m_c.gh = fval("ov-graph-h");
        m_c.gw = fval("ov-graph-w"); m_c.warn = fval("ov-warn"); m_c.pad = fval("ov-pad");
    }

    float refreshInterval() const {
        static constexpr float kRates[3] = {0.25f, 0.5f, 1.0f};
        return kRates[m_c.rate];
    }

    void refresh() {
        bool vis = m_c.enable && !(m_c.hideMenu && g_menuOpen);
        this->setVisible(vis);
        if (!vis) return;

        // Gather the most recent samples (newest first) without allocating.
        float tmp[kMax];
        int n = std::min(m_count, m_c.samples);
        float sum = 0.f, mnFt = 1e9f, mxFt = 0.f;
        for (int i = 0; i < n; ++i) {
            float v = m_ft[(m_head - 1 - i + kMax * 2) % kMax];
            tmp[i] = v; sum += v; mnFt = std::min(mnFt, v); mxFt = std::max(mxFt, v);
        }
        float fps = m_ema > 0.f ? 1.f / m_ema : 0.f;
        float avgFt = n ? sum / n : 0.f;

        std::string out;
        char buf[128];
        auto push = [&](const char* fmt, auto... a) {
            std::snprintf(buf, sizeof buf, fmt, a...);
            if (!out.empty()) out += '\n';
            out += buf;
        };
        int d = m_c.decimals;
        bool cp = m_c.compact;
        if (m_c.fps) {
            if (m_c.target) push(cp ? "%.*f/%.0f" : "FPS: %.*f / %.0f", d, fps, targetFps());
            else push(cp ? "%.*f" : "FPS: %.*f", d, fps);
        }
        if (m_c.ft) push(cp ? "%.*fms" : "Frame: %.*f ms", d + 1, m_ema * 1000.f);
        if (m_c.avg && n) push(cp ? "Av %.*f" : "Avg: %.*f", d, 1.f / avgFt);
        if (m_c.mn && n) push(cp ? "Mn %.*f" : "Min: %.*f", d, 1.f / mxFt);
        if (m_c.mx && n) push(cp ? "Mx %.*f" : "Max: %.*f", d, 1.f / mnFt);
        if (m_c.low1 && n) {
            float t2[kMax];
            std::copy(tmp, tmp + n, t2);
            int k = (int)std::ceil(0.99f * (n - 1));
            std::nth_element(t2, t2 + k, t2 + n);
            push(cp ? "1%% %.*f" : "1%% Low: %.*f", d, 1.f / t2[k]);
        }
        if (m_c.jit && n) {
            float var = 0.f;
            for (int i = 0; i < n; ++i) var += (tmp[i] - avgFt) * (tmp[i] - avgFt);
            push(cp ? "J %.*fms" : "Jitter: %.*f ms", d + 1, std::sqrt(var / n) * 1000.f);
        }
        if (m_c.clock || m_c.date) {
            std::time_t now = std::time(nullptr);
            std::tm lt{};
            localtime_r(&now, &lt);
            char tb[48];
            if (m_c.clock) {
                const char* f = m_c.h24 ? (m_c.secs ? "%H:%M:%S" : "%H:%M")
                                        : (m_c.secs ? "%I:%M:%S %p" : "%I:%M %p");
                std::strftime(tb, sizeof tb, f, &lt);
                push(cp ? "%s" : "Time: %s", tb);
            }
            if (m_c.date) {
                std::strftime(tb, sizeof tb, "%Y-%m-%d", &lt);
                push(cp ? "%s" : "Date: %s", tb);
            }
        }
        if (m_c.session) {
            int s = (int)m_session;
            push(cp ? "%02d:%02d:%02d" : "Session: %02d:%02d:%02d", s / 3600, (s / 60) % 60, s % 60);
        }
        if (m_c.frames) push(cp ? "#%llu" : "Frames: %llu", m_frames);
        if (m_c.res) {
            auto ws = CCDirector::get()->getWinSize();
            auto px = CCDirector::get()->getWinSizeInPixels();
            push(cp ? "%.0fx%.0f" : "Res: %.0fx%.0f (%.0fx%.0f px)", ws.width, ws.height, px.width, px.height);
        }
        if (m_c.nodes) {
            m_nodeAcc += refreshInterval();
            if (m_nodeAcc >= 1.f) { m_nodeAcc = 0.f; m_nodes = countNodes(CCDirector::get()->getRunningScene()); }
            push(cp ? "N %d" : "Nodes: %d", m_nodes);
        }
        if (m_c.children) {
            auto sc = CCDirector::get()->getRunningScene();
            push(cp ? "C %d" : "Children: %d", sc ? (int)sc->getChildrenCount() : 0);
        }
        if (out.empty()) out = " ";

        ccColor3B col = {255, 255, 255};
        if (m_c.color) {
            float tgt = targetFps();
            if (fps >= tgt * 0.9f) col = {110, 255, 110};
            else if (fps >= m_c.warn) col = {255, 225, 80};
            else col = {255, 90, 90};
        }
        m_label->setString(out.c_str());
        m_label->setScale(m_c.scale);
        m_label->setColor(col);
        m_label->setOpacity(pctByte(m_c.opacity));

        auto ls = m_label->getContentSize();
        float lw = ls.width * m_c.scale, lh = ls.height * m_c.scale;
        constexpr float in = 3.f;
        bool g = m_c.graph;
        float gw = g ? m_c.gw : 0.f, gh = g ? m_c.gh : 0.f;
        float W = std::max(lw, gw) + 2 * in;
        float H = lh + (g ? gh + in : 0.f) + 2 * in;
        m_bg->setContentSize({W, H});
        m_bg->setOpacity(pctByte(m_c.bgOpacity));
        m_bg->setVisible(m_c.bg);
        m_label->setPosition({in, H - in});

        m_graph->clear();
        if (g && n > 1) {
            float top = std::max(targetFps(), 1.f) * 1.25f;
            float stepX = gw / (n - 1);
            float a = m_c.opacity / 100.f;
            CCPoint prev;
            for (int k = 0; k < n; ++k) {
                float f = 1.f / tmp[n - 1 - k];
                CCPoint pt(in + k * stepX, in + std::clamp(f / top, 0.f, 1.f) * gh);
                if (k) m_graph->drawSegment(prev, pt, 0.5f, ccc4f(0.4f, 1.f, 0.6f, a));
                prev = pt;
            }
            m_graph->drawSegment(CCPoint(in, in), CCPoint(in + gw, in), 0.4f, ccc4f(1.f, 1.f, 1.f, 0.3f * a));
        }

        auto win = CCDirector::get()->getWinSize();
        float x = (m_c.corner % 2 == 0) ? m_c.pad : win.width - m_c.pad - W;
        float y = (m_c.corner < 2) ? win.height - m_c.pad - H : m_c.pad;
        this->setPosition({x, y});
    }

    void update(float dt) override {
        if (dt <= 0.f) return;
        m_session += dt;
        ++m_frames;
        if (!m_c.enable) return;
        m_ft[m_head] = dt;
        m_head = (m_head + 1) % kMax;
        m_count = std::min(m_count + 1, kMax);
        m_ema = m_ema == 0.f ? dt : m_ema * 0.9f + dt * 0.1f;
        m_acc += dt;
        if (m_acc >= refreshInterval()) { m_acc = 0.f; refresh(); }
    }

public:
    static StatsOverlay* create() {
        auto r = new StatsOverlay();
        if (r && r->init()) { r->autorelease(); return r; }
        CC_SAFE_DELETE(r);
        return nullptr;
    }
    static StatsOverlay* get() { return s_inst; }
    static StatsOverlay* ensure() {
        if (!s_inst) {
            s_inst = create();
            if (s_inst) {
                s_inst->retain(); // we own one reference for the lifetime of the game
                SceneManager::get()->keepAcrossScenes(s_inst);
            }
        }
        return s_inst;
    }
    // Re-read settings and redraw. Safe to call when no overlay exists.
    static void reloadAll() { if (s_inst) s_inst->reload(); }
    static void resetAll() { if (s_inst) s_inst->resetStats(); }

    void reload() {
        readCfg();
        if (!m_label || m_builtFont != m_c.font) rebuildLabel();
        m_acc = 0.f;
        this->refresh();
    }
    void resetStats() {
        m_ft.fill(0.f); m_head = 0; m_count = 0; m_ema = 0.f; m_session = 0.0; m_frames = 0;
        this->refresh();
    }
    double sessionSeconds() const { return m_session; }
    unsigned long long frames() const { return m_frames; }
};

} // namespace mm
