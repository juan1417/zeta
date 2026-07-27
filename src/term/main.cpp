// Zeta Term - Renderer nativo de terminal (sin OpenGL)
// Consume scene JSON y lo imprime con ANSI 24-bit

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <fstream>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include "deps/json.hpp"

using json = nlohmann::json;

struct SceneNode {
    std::string tipo;
    std::string titulo;
    std::string id;
    std::map<std::string, std::string> cols;
    std::map<std::string, double> nums;
    std::map<std::string, std::string> strs;
    std::map<std::string, std::vector<double>> series;
};

struct SceneLayout {
    std::string tipo = "grid";
    int cols = 2;
    int gap = 2;
    std::string background = "#1e1e1e";
};

struct Scene {
    std::string titulo;
    std::string autor;
    SceneLayout layout;
    std::vector<SceneNode> nodes;
};

static Scene parse_scene(const json& j) {
    Scene s;
    s.titulo = j.value("titulo", "");
    s.autor = j.value("autor", "");
    if (j.contains("layout") && j["layout"].is_object()) {
        const auto& l = j["layout"];
        s.layout.tipo = l.value("tipo", "grid");
        s.layout.cols = l.value("cols", 2);
        s.layout.gap = l.value("gap", 2);
        s.layout.background = l.value("background", "#1e1e1e");
    }
    if (j.contains("nodes") && j["nodes"].is_array()) {
        for (const auto& nj : j["nodes"]) {
            SceneNode n;
            n.tipo = nj.value("tipo", "unknown");
            n.titulo = nj.value("titulo", "");
            n.id = nj.value("id", "");
            if (nj.contains("cols") && nj["cols"].is_object())
                for (auto it = nj["cols"].begin(); it != nj["cols"].end(); ++it)
                    n.cols[it.key()] = it.value().get<std::string>();
            if (nj.contains("nums") && nj["nums"].is_object())
                for (auto it = nj["nums"].begin(); it != nj["nums"].end(); ++it)
                    n.nums[it.key()] = it.value().get<double>();
            if (nj.contains("strs") && nj["strs"].is_object())
                for (auto it = nj["strs"].begin(); it != nj["strs"].end(); ++it)
                    n.strs[it.key()] = it.value().get<std::string>();
            if (nj.contains("data") && nj["data"].is_object()) {
                for (auto it = nj["data"].begin(); it != nj["data"].end(); ++it) {
                    std::vector<double> v;
                    for (const auto& x : it.value()) {
                        if (x.is_null()) v.push_back(std::nan(""));
                        else if (x.is_number()) v.push_back(x.get<double>());
                        else v.push_back(std::nan(""));
                    }
                    n.series[it.key()] = std::move(v);
                }
            }
            s.nodes.push_back(std::move(n));
        }
    }
    return s;
}

static bool g_color = true;

struct RGB { int r, g, b; };

static std::string ansi_fg(RGB c) {
    if (!g_color) return "";
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[38;2;%d;%d;%dm", c.r, c.g, c.b);
    return buf;
}
static std::string ansi_bg(RGB c) {
    if (!g_color) return "";
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[48;2;%d;%d;%dm", c.r, c.g, c.b);
    return buf;
}
static std::string ansi_reset() {
    if (!g_color) return "";
    return "\x1b[0m";
}
static RGB hex_to_rgb(const std::string& hex, RGB fallback = {100, 180, 255}) {
    if (hex.size() < 7 || hex[0] != '#') return fallback;
    RGB c;
    sscanf(hex.c_str() + 1, "%02x%02x%02x", &c.r, &c.g, &c.b);
    return c;
}

static int detect_width() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        int cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        if (cols > 0) return cols;
    }
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) return (int)w.ws_col;
#endif
    const char* cols = std::getenv("COLUMNS");
    if (cols) { int n = std::atoi(cols); if (n > 0) return n; }
    return 120;
}

struct Cell {
    char ch = ' ';
    RGB fg = {200, 200, 200};
    RGB bg = {30, 30, 30};
};

struct Canvas {
    int w, h;
    std::vector<Cell> cells;
    Canvas(int W, int H) : w(W), h(H), cells(W * H) {
        for (auto& c : cells) { c.ch = ' '; c.fg = {200, 200, 200}; c.bg = {30, 30, 30}; }
    }
    Cell& at(int x, int y) { return cells[y * w + x]; }
    const Cell& at(int x, int y) const { return cells[y * w + x]; }
    void put(int x, int y, char ch, RGB fg, RGB bg) {
        if (x < 0 || x >= w || y < 0 || y >= h) return;
        auto& c = at(x, y);
        c.ch = ch; c.fg = fg; c.bg = bg;
    }
    void set_bg(int x, int y, RGB bg) {
        if (x < 0 || x >= w || y < 0 || y >= h) return;
        at(x, y).bg = bg;
    }
    void hline(int x0, int x1, int y, char ch, RGB fg, RGB bg) {
        for (int x = x0; x <= x1; ++x) put(x, y, ch, fg, bg);
    }
    void vline(int x, int y0, int y1, char ch, RGB fg, RGB bg) {
        for (int y = y0; y <= y1; ++y) put(x, y, ch, fg, bg);
    }
    void rect_bg(int x0, int y0, int x1, int y1, RGB bg) {
        for (int y = y0; y <= y1; ++y)
            for (int x = x0; x <= x1; ++x) set_bg(x, y, bg);
    }
};

static std::string canvas_to_ansi(const Canvas& c) {
    std::string out;
    out.reserve(c.w * c.h * 30);
    RGB cur_fg = {-1, -1, -1};
    RGB cur_bg = {-1, -1, -1};
    for (int y = 0; y < c.h; ++y) {
        for (int x = 0; x < c.w; ++x) {
            const Cell& cell = c.at(x, y);
            bool nf = (cell.fg.r != cur_fg.r || cell.fg.g != cur_fg.g || cell.fg.b != cur_fg.b);
            bool nb = (cell.bg.r != cur_bg.r || cell.bg.g != cur_bg.g || cell.bg.b != cur_bg.b);
            if (nf || nb) {
                if (g_color) {
                    char buf[64];
                    if (nf && nb) {
                        snprintf(buf, sizeof(buf), "\x1b[38;2;%d;%d;%d;48;2;%d;%d;%dm",
                                 cell.fg.r, cell.fg.g, cell.fg.b, cell.bg.r, cell.bg.g, cell.bg.b);
                    } else if (nf) {
                        snprintf(buf, sizeof(buf), "\x1b[38;2;%d;%d;%dm", cell.fg.r, cell.fg.g, cell.fg.b);
                    } else {
                        snprintf(buf, sizeof(buf), "\x1b[48;2;%d;%d;%dm", cell.bg.r, cell.bg.g, cell.bg.b);
                    }
                    out += buf;
                }
                cur_fg = cell.fg;
                cur_bg = cell.bg;
            }
            out += cell.ch ? cell.ch : ' ';
        }
        out += "\x1b[0m\n";
        cur_fg = cur_bg = {-1, -1, -1};
    }
    return out;
}

static void data_range(const std::vector<double>& v, double& mn, double& mx) {
    mn = std::nan(""); mx = std::nan("");
    for (double x : v) {
        if (std::isnan(x)) continue;
        if (std::isnan(mn) || x < mn) mn = x;
        if (std::isnan(mx) || x > mx) mx = x;
    }
    if (std::isnan(mn)) { mn = 0; mx = 1; }
    if (mn == mx) { mn -= 0.5; mx += 0.5; }
}

static void render_metric(Canvas& c, int x0, int y0, int w, int h, const SceneNode& n) {
    if (w < 4 || h < 3) return;
    RGB val_color = hex_to_rgb(n.strs.count("color") ? n.strs.at("color") : "#64b4ff");
    double val = n.nums.count("value") ? n.nums.at("value") : 0.0;
    std::string unit = n.strs.count("unit") ? n.strs.at("unit") : "";
    char buf[64];
    snprintf(buf, sizeof(buf), "%.2f %s", val, unit.c_str());
    std::string val_str = buf;
    int val_x = x0 + (w - (int)val_str.size()) / 2;
    int val_y = y0 + h / 2;
    for (size_t i = 0; i < val_str.size(); ++i)
        c.put(val_x + i, val_y, val_str[i], val_color, {30, 30, 30});
}

static void render_text(Canvas& c, int x0, int y0, int w, int h, const SceneNode& n) {
    std::string text = n.strs.count("text") ? n.strs.at("text") : n.titulo;
    for (int y = y0; y < y0 + h; ++y) {
        int line_idx = y - y0;
        int start = line_idx * (w - 1);
        for (int x = x0; x < x0 + w - 1 && start + (x - x0) < (int)text.size(); ++x)
            c.put(x, y, text[start + (x - x0)], {200, 200, 200}, {30, 30, 30});
    }
}

static void render_line_plot(Canvas& c, int x0, int y0, int w, int h, const SceneNode& n) {
    if (w < 4 || h < 4) return;
    std::string xk = n.cols.count("x") ? n.cols.at("x") : "";
    std::string yk = n.cols.count("y") ? n.cols.at("y") : "";
    if (xk.empty() || yk.empty() || !n.series.count(xk) || !n.series.count(yk)) return;
    const auto& xs = n.series.at(xk);
    const auto& ys = n.series.at(yk);
    int npts = std::min((int)xs.size(), (int)ys.size());
    if (npts == 0) return;
    double ymn, ymx; data_range(ys, ymn, ymx);
    double xmn, xmx; data_range(xs, xmn, xmx);
    int plot_x0 = x0 + 2;
    int plot_y0 = y0;
    int plot_w = w - 2;
    int plot_h = h - 2;
    if (plot_w < 2 || plot_h < 2) return;
    RGB line_color = {100, 180, 255};
    auto plot_x = [&](int i) { return plot_x0 + (int)((xs[i] - xmn) / (xmx - xmn) * (plot_w - 1)); };
    auto plot_y = [&](int i) { return plot_y0 + (int)((ymx - ys[i]) / (ymx - ymn) * (plot_h - 1)); };
    for (int i = 1; i < npts; ++i) {
        int x1 = plot_x(i - 1), y1 = plot_y(i - 1);
        int x2 = plot_x(i), y2 = plot_y(i);
        int dx = std::abs(x2 - x1), dy = std::abs(y2 - y1);
        int sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1;
        int err = dx - dy;
        int x = x1, y = y1;
        while (true) {
            c.put(x, y, '*', line_color, {30, 30, 30});
            if (x == x2 && y == y2) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x += sx; }
            if (e2 < dx) { err += dx; y += sy; }
        }
    }
}

static void render_bar_chart(Canvas& c, int x0, int y0, int w, int h, const SceneNode& n) {
    if (w < 4 || h < 4) return;
    std::string yk = n.cols.count("y") ? n.cols.at("y") : "";
    if (yk.empty() || !n.series.count(yk)) return;
    const auto& ys = n.series.at(yk);
    if (ys.empty()) return;
    double ymn, ymx; data_range(ys, ymn, ymx);
    int plot_x0 = x0;
    int plot_y0 = y0;
    int plot_w = w;
    int plot_h = h - 1;
    if (plot_w < 2 || plot_h < 2) return;
    int bar_w = std::max(1, plot_w / (int)ys.size());
    RGB bar_color = {100, 180, 255};
    for (size_t i = 0; i < ys.size(); ++i) {
        double v = ys[i];
        if (std::isnan(v)) continue;
        int bar_h = (int)((v - ymn) / (ymx - ymn) * plot_h);
        if (bar_h < 0) bar_h = 0;
        if (bar_h > plot_h) bar_h = plot_h;
        int bx = plot_x0 + (int)i * bar_w;
        for (int dy = 0; dy < bar_h; ++dy) {
            int y = plot_y0 + plot_h - 1 - dy;
            for (int dx = 0; dx < bar_w; ++dx) {
                if (bx + dx < x0 + w) c.put(bx + dx, y, '#', bar_color, {30, 30, 30});
            }
        }
    }
}

static void render_scatter(Canvas& c, int x0, int y0, int w, int h, const SceneNode& n) {
    if (w < 4 || h < 4) return;
    std::string xk = n.cols.count("x") ? n.cols.at("x") : "";
    std::string yk = n.cols.count("y") ? n.cols.at("y") : "";
    if (xk.empty() || yk.empty() || !n.series.count(xk) || !n.series.count(yk)) return;
    const auto& xs = n.series.at(xk);
    const auto& ys = n.series.at(yk);
    int npts = std::min((int)xs.size(), (int)ys.size());
    if (npts == 0) return;
    double ymn, ymx; data_range(ys, ymn, ymx);
    double xmn, xmx; data_range(xs, xmn, xmx);
    int plot_x0 = x0 + 1;
    int plot_y0 = y0;
    int plot_w = w - 1;
    int plot_h = h - 1;
    if (plot_w < 1 || plot_h < 1) return;
    RGB pt_color = {100, 180, 255};
    for (int i = 0; i < npts; ++i) {
        if (std::isnan(xs[i]) || std::isnan(ys[i])) continue;
        int px = plot_x0 + (int)((xs[i] - xmn) / (xmx - xmn) * (plot_w - 1));
        int py = plot_y0 + (int)((ymx - ys[i]) / (ymx - ymn) * (plot_h - 1));
        c.put(px, py, '*', pt_color, {30, 30, 30});
    }
}

static void render_histogram(Canvas& c, int x0, int y0, int w, int h, const SceneNode& n) {
    if (w < 4 || h < 4) return;
    std::string yk = n.cols.count("y") ? n.cols.at("y") : "";
    if (yk.empty() || !n.series.count(yk)) return;
    const auto& ys = n.series.at(yk);
    if (ys.empty()) return;
    int bins = n.nums.count("bins") ? (int)n.nums.at("bins") : 10;
    double ymn, ymx; data_range(ys, ymn, ymx);
    std::vector<int> counts(bins, 0);
    for (double v : ys) {
        if (std::isnan(v)) continue;
        int b = (int)((v - ymn) / (ymx - ymn) * bins);
        if (b == bins) --b;
        if (b >= 0 && b < bins) counts[b]++;
    }
    int mx = 0;
    for (int cc : counts) if (cc > mx) mx = cc;
    if (mx == 0) return;
    int plot_x0 = x0;
    int plot_y0 = y0;
    int plot_w = w;
    int plot_h = h - 1;
    if (plot_w < 2 || plot_h < 2) return;
    int bar_w = std::max(1, plot_w / bins);
    RGB bar_color = {100, 180, 255};
    for (int b = 0; b < bins; ++b) {
        int bh = (int)((double)counts[b] / mx * plot_h);
        int bx = plot_x0 + b * bar_w;
        for (int dy = 0; dy < bh; ++dy) {
            int y = plot_y0 + plot_h - 1 - dy;
            for (int dx = 0; dx < bar_w && bx + dx < x0 + w; ++dx)
                c.put(bx + dx, y, '#', bar_color, {30, 30, 30});
        }
    }
}

static void render_box_plot(Canvas& c, int x0, int y0, int w, int h, const SceneNode& n) {
    if (w < 6 || h < 4) return;
    std::string yk = n.cols.count("y") ? n.cols.at("y") : "";
    if (yk.empty() || !n.series.count(yk)) return;
    const std::vector<double>& ys = n.series.at(yk);
    if (ys.empty()) return;
    std::vector<double> sorted = ys;
    std::sort(sorted.begin(), sorted.end());
    auto pct = [&](double p) -> double {
        if (sorted.empty()) return 0;
        size_t idx = (size_t)(p * (sorted.size() - 1));
        return sorted[std::min(idx, sorted.size() - 1)];
    };
    double mn = sorted.front();
    double mx = sorted.back();
    double q1 = pct(0.25);
    double med = pct(0.5);
    double q3 = pct(0.75);
    int plot_x0 = x0 + 1;
    int plot_y0 = y0;
    int plot_w = w - 1;
    int plot_h = h - 1;
    if (plot_w < 4 || plot_h < 4) return;
    double ymn = mn, ymx = mx;
    if (ymn == ymx) { ymn -= 0.5; ymx += 0.5; }
    auto to_y = [&](double v) { return plot_y0 + (int)((ymx - v) / (ymx - ymn) * (plot_h - 1)); };
    int cx = plot_x0 + plot_w / 2;
    int half_box = std::max(1, plot_w / 6);
    RGB box_color = {100, 180, 255};
    RGB med_color = {255, 200, 100};
    int ymn_y = to_y(mn), ymx_y = to_y(mx);
    c.put(cx, ymn_y, '+', box_color, {30, 30, 30});
    c.put(cx, ymx_y, '+', box_color, {30, 30, 30});
    c.vline(cx, ymn_y, ymx_y, '|', {80, 80, 80}, {30, 30, 30});
    int q1_y = to_y(q1), q3_y = to_y(q3);
    c.rect_bg(cx - half_box, q3_y, cx + half_box, q1_y, {40, 40, 60});
    c.hline(cx - half_box, cx + half_box, q1_y, '-', box_color, {30, 30, 30});
    c.hline(cx - half_box, cx + half_box, q3_y, '-', box_color, {30, 30, 30});
    c.vline(cx - half_box, q1_y, q3_y, '|', box_color, {30, 30, 30});
    c.vline(cx + half_box, q1_y, q3_y, '|', box_color, {30, 30, 30});
    c.hline(cx - half_box, cx + half_box, to_y(med), '=', med_color, {30, 30, 30});
}

static void render_linear_regression(Canvas& c, int x0, int y0, int w, int h, const SceneNode& n) {
    if (w < 4 || h < 4) return;
    std::string xk = n.cols.count("x") ? n.cols.at("x") : "";
    std::string yk = n.cols.count("y") ? n.cols.at("y") : "";
    if (xk.empty() || yk.empty() || !n.series.count(xk) || !n.series.count(yk)) return;
    const auto& xs = n.series.at(xk);
    const auto& ys = n.series.at(yk);
    int npts = std::min((int)xs.size(), (int)ys.size());
    if (npts == 0) return;
    double ymn, ymx; data_range(ys, ymn, ymx);
    double xmn, xmx; data_range(xs, xmn, xmx);
    double mx_ = 0, my_ = 0;
    int nn = 0;
    for (int i = 0; i < npts; ++i) {
        if (std::isnan(xs[i]) || std::isnan(ys[i])) continue;
        mx_ += xs[i]; my_ += ys[i]; ++nn;
    }
    if (nn == 0) return;
    mx_ /= nn; my_ /= nn;
    double num = 0, den = 0;
    for (int i = 0; i < npts; ++i) {
        if (std::isnan(xs[i]) || std::isnan(ys[i])) continue;
        num += (xs[i] - mx_) * (ys[i] - my_);
        den += (xs[i] - mx_) * (xs[i] - mx_);
    }
    double slope = den != 0 ? num / den : 0;
    double intercept = my_ - slope * mx_;
    int plot_x0 = x0 + 1;
    int plot_y0 = y0;
    int plot_w = w - 1;
    int plot_h = h - 1;
    if (plot_w < 2 || plot_h < 2) return;
    RGB line_color = {255, 150, 100};
    for (int px = 0; px < plot_w; ++px) {
        double x_val = xmn + (double)px / (plot_w - 1) * (xmx - xmn);
        double y_val = slope * x_val + intercept;
        int py = (int)((ymx - y_val) / (ymx - ymn) * (plot_h - 1));
        if (py >= 0 && py < plot_h) c.put(plot_x0 + px, plot_y0 + py, '-', line_color, {30, 30, 30});
    }
    RGB pt_color = {100, 180, 255};
    for (int i = 0; i < npts; ++i) {
        if (std::isnan(xs[i]) || std::isnan(ys[i])) continue;
        int px = (int)((xs[i] - xmn) / (xmx - xmn) * (plot_w - 1));
        int py = (int)((ymx - ys[i]) / (ymx - ymn) * (plot_h - 1));
        c.put(plot_x0 + px, plot_y0 + py, '*', pt_color, {30, 30, 30});
    }
}

static void render_node(Canvas& c, int x0, int y0, int w, int h, const SceneNode& n) {
    if (n.tipo == "metric") render_metric(c, x0, y0, w, h, n);
    else if (n.tipo == "text") render_text(c, x0, y0, w, h, n);
    else if (n.tipo == "line_plot") render_line_plot(c, x0, y0, w, h, n);
    else if (n.tipo == "bar_chart") render_bar_chart(c, x0, y0, w, h, n);
    else if (n.tipo == "scatter") render_scatter(c, x0, y0, w, h, n);
    else if (n.tipo == "histogram") render_histogram(c, x0, y0, w, h, n);
    else if (n.tipo == "box_plot") render_box_plot(c, x0, y0, w, h, n);
    else if (n.tipo == "linear_regression") render_linear_regression(c, x0, y0, w, h, n);
    else c.put(x0, y0, '?', {255, 100, 100}, {30, 30, 30});
}

static void render_layout(Canvas& c, const Scene& s) {
    if (s.nodes.empty()) {
        std::string msg = "Sin nodos. Carga una scene via --file o conecta al server.";
        for (size_t i = 0; i < msg.size() && (int)i < c.w; ++i)
            c.put(2 + (int)i, 1, msg[i], {150, 150, 150}, {30, 30, 30});
        return;
    }
    int cols = std::max(1, s.layout.cols);
    int gap = 2;
    int total_gap = gap * (cols - 1);
    int cell_w = (c.w - total_gap) / cols;
    int cell_h = 14;
    RGB border_color = {60, 60, 60};
    RGB bg_color = {30, 30, 30};
    RGB title_color = {220, 220, 220};

    for (size_t i = 0; i < s.nodes.size(); ++i) {
        const auto& n = s.nodes[i];
        int col = i % cols;
        int row = i / cols;
        int cx = col * (cell_w + gap);
        int cy = row * (cell_h + 1);

        // Top border with title
        c.put(cx, cy, '+', border_color, bg_color);
        std::string t = n.titulo;
        if ((int)t.size() > cell_w - 2) t = t.substr(0, cell_w - 2);
        int tx = cx + 1;
        for (size_t j = 0; j < t.size(); ++j) c.put(tx + j, cy, t[j], title_color, bg_color);
        int rest_x = tx + (int)t.size();
        for (int x = rest_x; x < cx + cell_w; ++x) c.put(x, cy, '-', border_color, bg_color);
        c.put(cx + cell_w, cy, '+', border_color, bg_color);

        // Side borders
        for (int y = cy + 1; y < cy + cell_h; ++y) {
            c.put(cx, y, '|', border_color, bg_color);
            c.put(cx + cell_w, y, '|', border_color, bg_color);
        }

        // Bottom border
        c.put(cx, cy + cell_h, '+', border_color, bg_color);
        for (int x = cx + 1; x < cx + cell_w; ++x) c.put(x, cy + cell_h, '-', border_color, bg_color);
        c.put(cx + cell_w, cy + cell_h, '+', border_color, bg_color);

        // Content
        render_node(c, cx + 1, cy + 1, cell_w - 1, cell_h - 1, n);
    }
}

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

static std::string http_get(const std::string& host, int port, const std::string& path) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return "";
    struct hostent* he = gethostbyname(host.c_str());
    if (!he) {
#ifdef _WIN32
        closesocket(sock); WSACleanup();
#else
        close(sock);
#endif
        return "";
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
#ifdef _WIN32
        closesocket(sock); WSACleanup();
#else
        close(sock);
#endif
        return "";
    }
    std::string req = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
    send(sock, req.c_str(), req.size(), 0);
    std::string resp;
    char buf[4096];
    int n;
    while ((n = recv(sock, buf, sizeof(buf), 0)) > 0) resp.append(buf, n);
#ifdef _WIN32
    closesocket(sock); WSACleanup();
#else
    close(sock);
#endif
    size_t pos = resp.find("\r\n\r\n");
    if (pos == std::string::npos) return "";
    return resp.substr(pos + 4);
}

int main(int argc, char* argv[]) {
    std::string host = "localhost";
    int port = 8080;
    std::string scene_file;
    int forced_width = 0;
    int node_height = 14;
    int cell_h = node_height;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--help" || a == "-h") {
            std::printf("Zeta Term - Renderer ANSI de terminal\n\n");
            std::printf("Uso: zeta_term [opciones]\n\n");
            std::printf("Opciones:\n");
            std::printf("  --help, -h              Mostrar esta ayuda\n");
            std::printf("  --host HOST             Host del server (default: localhost)\n");
            std::printf("  --port PORT             Puerto del server (default: 8080)\n");
            std::printf("  --file PATH             Leer escena desde archivo JSON en vez del server\n");
            std::printf("  --width WIDTH           Ancho forzado de la terminal\n");
            std::printf("  --no-color              Deshabilitar colores ANSI\n");
            std::printf("\nEjemplos:\n");
            std::printf("  zeta_term                              Conectar al server por defecto\n");
            std::printf("  zeta_term --host 192.168.1.10 --port 9000\n");
            std::printf("  zeta_term --file scene.json            Renderizar desde archivo\n");
            return 0;
        }
        else if (a == "--host" && i + 1 < argc) host = argv[++i];
        else if (a == "--port" && i + 1 < argc) port = std::atoi(argv[++i]);
        else if (a == "--file" && i + 1 < argc) scene_file = argv[++i];
        else if (a == "--width" && i + 1 < argc) forced_width = std::atoi(argv[++i]);
        else if (a == "--no-color") g_color = false;
        else {
            std::fprintf(stderr, "Opcion desconocida: %s\nUsa --help para ver las opciones.\n", a.c_str());
            return 1;
        }
    }

    int W = forced_width > 0 ? forced_width : detect_width();

    std::string body;
    if (!scene_file.empty()) {
        std::ifstream f(scene_file);
        if (!f.good()) { std::fprintf(stderr, "Error: no se pudo abrir %s\n", scene_file.c_str()); return 1; }
        std::stringstream ss; ss << f.rdbuf();
        body = ss.str();
    } else {
        body = http_get(host, port, "/api/grafo");
        if (body.empty()) {
            std::fprintf(stderr, "Error: no se pudo conectar a http://%s:%d/api/grafo\n", host.c_str(), port);
            return 1;
        }
    }

    Scene scene;
    try {
        auto j = json::parse(body);
        scene = parse_scene(j);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Error parseando JSON: %s\n", e.what());
        return 1;
    }

    int cols = std::max(1, scene.layout.cols);
    int rows = (scene.nodes.empty() ? 1 : ((int)scene.nodes.size() + cols - 1) / cols);
    int H = rows * (cell_h + 1) + 1;

    RGB bg_color = hex_to_rgb(scene.layout.background, {30, 30, 30});
    Canvas c(W, H);
    for (auto& cell : c.cells) cell.bg = bg_color;

    // Top title bar
    char title[256];
    snprintf(title, sizeof(title), " Zeta Dashboard  -  %s  (%s, %zu nodos) ",
             scene.titulo.c_str(), scene.autor.c_str(), scene.nodes.size());
    for (int x = 0; x < W; ++x) c.put(x, 0, ' ', {30, 30, 30}, {50, 50, 50});
    for (size_t i = 0; i < strlen(title) && (int)i < W; ++i)
        c.put((int)i, 0, title[i], {200, 200, 200}, {50, 50, 50});

    // Layout in remaining rows (shift down by 1)
    Canvas layout_canvas(W, H - 1);
    for (auto& cell : layout_canvas.cells) cell.bg = bg_color;
    render_layout(layout_canvas, scene);

    // Copy layout_canvas into c at row 1+
    for (int y = 0; y < H - 1; ++y)
        for (int x = 0; x < W; ++x) {
            const Cell& lc = layout_canvas.at(x, y);
            Cell& tc = c.at(x, y + 1);
            if (lc.ch != ' ' || lc.bg.r != bg_color.r || lc.bg.g != bg_color.g || lc.bg.b != bg_color.b) {
                tc = lc;
            }
        }

    std::string out = canvas_to_ansi(c);
    fwrite(out.data(), 1, out.size(), stdout);
    if (g_color) fwrite("\x1b[0m", 1, 4, stdout);
    return 0;
}
