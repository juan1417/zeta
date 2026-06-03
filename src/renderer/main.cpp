// Zeta Dashboard Renderer
// Native OpenGL + GLFW + ImGui + ImPlot renderer
// Consumes JSON scene spec from /api/grafo endpoint

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

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
    int gap = 10;
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
        s.layout.gap = l.value("gap", 10);
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

static ImU32 hex_to_color(const std::string& hex, ImU32 fallback = IM_COL32(100, 180, 255, 255)) {
    if (hex.empty() || hex[0] != '#') return fallback;
    unsigned int r = 100, g = 180, b = 255, a = 255;
    if (hex.size() == 7) {
        sscanf(hex.c_str() + 1, "%02x%02x%02x", &r, &g, &b);
    } else if (hex.size() == 9) {
        sscanf(hex.c_str() + 1, "%02x%02x%02x%02x", &r, &g, &b, &a);
    }
    return IM_COL32(r, g, b, a);
}

static ImVec4 hex_to_vec4(const std::string& hex, ImVec4 fallback = ImVec4(0.39f, 0.71f, 1.0f, 1.0f)) {
    if (hex.empty() || hex[0] != '#') return fallback;
    unsigned int r, g, b;
    if (hex.size() == 7) {
        sscanf(hex.c_str() + 1, "%02x%02x%02x", &r, &g, &b);
        return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    }
    return fallback;
}

static double compute_mean(const std::vector<double>& v) {
    double s = 0;
    int n = 0;
    for (double x : v) { if (!std::isnan(x)) { s += x; ++n; } }
    return n > 0 ? s / n : std::nan("");
}

static double compute_stddev(const std::vector<double>& v) {
    double m = compute_mean(v);
    double s = 0;
    int n = 0;
    for (double x : v) { if (!std::isnan(x)) { s += (x - m) * (x - m); ++n; } }
    return n > 0 ? std::sqrt(s / n) : std::nan("");
}

static void linear_regression(const std::vector<double>& xs, const std::vector<double>& ys,
                              double& slope, double& intercept) {
    double mx = compute_mean(xs);
    double my = compute_mean(ys);
    double num = 0, den = 0;
    int n = std::min(xs.size(), ys.size());
    for (int i = 0; i < n; ++i) {
        if (std::isnan(xs[i]) || std::isnan(ys[i])) continue;
        num += (xs[i] - mx) * (ys[i] - my);
        den += (xs[i] - mx) * (xs[i] - mx);
    }
    slope = den != 0 ? num / den : 0;
    intercept = my - slope * mx;
}

// ============ Renderers por nodo ============

static void render_metric(const SceneNode& n) {
    ImGui::BeginGroup();
    ImVec4 col = hex_to_vec4(n.strs.count("color") ? n.strs.at("color") : "#64b4ff");
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);

    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", n.titulo.c_str());
    ImGui::Spacing();

    double val = n.nums.count("value") ? n.nums.at("value") : 0.0;
    std::string unit = n.strs.count("unit") ? n.strs.at("unit") : "";
    char buf[64];
    snprintf(buf, sizeof(buf), "%.2f %s", val, unit.c_str());
    ImGui::TextColored(col, "%s", buf);

    ImGui::PopFont();
    ImGui::EndGroup();
}

static void render_text(const SceneNode& n) {
    ImGui::TextWrapped("%s", n.strs.count("text") ? n.strs.at("text").c_str() : n.titulo.c_str());
}

static const std::vector<double>& get_col(const SceneNode& n, const std::string& key, const std::vector<double>* def = nullptr) {
    auto it = n.series.find(key);
    if (it == n.series.end()) {
        static const std::vector<double> empty;
        return def ? *def : empty;
    }
    return it->second;
}

static void render_line_plot(const SceneNode& n) {
    std::string xk = n.cols.count("x") ? n.cols.at("x") : "";
    std::string yk = n.cols.count("y") ? n.cols.at("y") : "";
    if (xk.empty() || yk.empty() || !n.series.count(xk) || !n.series.count(yk)) {
        ImGui::TextDisabled("line_plot: faltan cols x/y");
        return;
    }
    const auto& xs = n.series.at(xk);
    const auto& ys = n.series.at(yk);
    int npts = std::min(xs.size(), ys.size());

    if (ImPlot::BeginPlot(n.titulo.c_str(), ImVec2(-1, 280))) {
        ImPlot::SetupAxes(xk.c_str(), yk.c_str());
        ImPlot::PlotLine(n.titulo.c_str(), xs.data(), ys.data(), npts);
        ImPlot::EndPlot();
    }
}

static void render_bar_chart(const SceneNode& n) {
    std::string xk = n.cols.count("x") ? n.cols.at("x") : "";
    std::string yk = n.cols.count("y") ? n.cols.at("y") : "";
    if (xk.empty() || yk.empty() || !n.series.count(xk) || !n.series.count(yk)) {
        ImGui::TextDisabled("bar_chart: faltan cols x/y");
        return;
    }
    const auto& xs = n.series.at(xk);
    const auto& ys = n.series.at(yk);
    int npts = std::min(xs.size(), ys.size());

    if (ImPlot::BeginPlot(n.titulo.c_str(), ImVec2(-1, 280))) {
        ImPlot::SetupAxes(xk.c_str(), yk.c_str());
        ImPlot::PlotBars(n.titulo.c_str(), ys.data(), npts);
        ImPlot::EndPlot();
    }
}

static void render_scatter(const SceneNode& n) {
    std::string xk = n.cols.count("x") ? n.cols.at("x") : "";
    std::string yk = n.cols.count("y") ? n.cols.at("y") : "";
    if (xk.empty() || yk.empty() || !n.series.count(xk) || !n.series.count(yk)) {
        ImGui::TextDisabled("scatter: faltan cols x/y");
        return;
    }
    const auto& xs = n.series.at(xk);
    const auto& ys = n.series.at(yk);
    int npts = std::min(xs.size(), ys.size());

    if (ImPlot::BeginPlot(n.titulo.c_str(), ImVec2(-1, 280))) {
        ImPlot::SetupAxes(xk.c_str(), yk.c_str());
        ImPlot::PlotScatter(n.titulo.c_str(), xs.data(), ys.data(), npts);
        ImPlot::EndPlot();
    }
}

static void render_heatmap(const SceneNode& n) {
    std::string xk = n.cols.count("x") ? n.cols.at("x") : "";
    std::string yk = n.cols.count("y") ? n.cols.at("y") : "";
    if (xk.empty() || yk.empty() || !n.series.count(xk) || !n.series.count(yk)) {
        ImGui::TextDisabled("heatmap: faltan cols x/y");
        return;
    }
    const auto& xs = n.series.at(xk);
    const auto& ys = n.series.at(yk);
    int npts = std::min(xs.size(), ys.size());
    ImGui::Text("Heatmap: %d puntos (%s vs %s)", npts, xk.c_str(), yk.c_str());
    if (ImPlot::BeginPlot(n.titulo.c_str(), ImVec2(-1, 280))) {
        ImPlot::SetupAxes(xk.c_str(), yk.c_str());
        ImPlot::PlotScatter(n.titulo.c_str(), xs.data(), ys.data(), npts);
        ImPlot::EndPlot();
    }
}

static void render_histogram(const SceneNode& n) {
    std::string yk = n.cols.count("y") ? n.cols.at("y") : "";
    if (yk.empty() || !n.series.count(yk)) {
        ImGui::TextDisabled("histogram: falta col y");
        return;
    }
    const auto& ys = n.series.at(yk);
    int bins = n.nums.count("bins") ? (int)n.nums.at("bins") : 10;

    if (ImPlot::BeginPlot(n.titulo.c_str(), ImVec2(-1, 280))) {
        ImPlot::SetupAxes(yk.c_str(), "Frecuencia");
        ImPlot::PlotHistogram(n.titulo.c_str(), ys.data(), (int)ys.size(), bins);
        ImPlot::EndPlot();
    }
}

static void render_box_plot(const SceneNode& n) {
    std::string yk = n.cols.count("y") ? n.cols.at("y") : "";
    if (yk.empty() || !n.series.count(yk)) {
        ImGui::TextDisabled("box_plot: falta col y");
        return;
    }
    const auto& ys = n.series.at(yk);

    // Compute box plot stats: min, q1, median, q3, max
    std::vector<double> sorted = ys;
    std::sort(sorted.begin(), sorted.end());
    auto pct = [&](double p) -> double {
        if (sorted.empty()) return std::nan("");
        size_t idx = (size_t)(p * (sorted.size() - 1));
        return sorted[idx];
    };
    double mn = sorted.empty() ? 0 : sorted.front();
    double mx = sorted.empty() ? 0 : sorted.back();
    double q1 = pct(0.25);
    double med = pct(0.5);
    double q3 = pct(0.75);

    if (ImPlot::BeginPlot(n.titulo.c_str(), ImVec2(-1, 280))) {
        ImPlot::SetupAxes("", yk.c_str());
        // Whiskers: min y max
        double xs[3] = {0.0, 0.5, 1.0};
        double lows[3] = {mn, q1, q3};
        double centers[3] = {q1, med, mx};
        double highs[3] = {q1, q3, mx};
        ImPlot::PlotErrorBars("whiskers", xs, centers, lows, highs, 3);
        // Median line
        double lx[2] = {0.0, 1.0};
        double ly[2] = {med, med};
        ImPlot::PlotLine("median", lx, ly, 2);
        ImPlot::EndPlot();
    }
    ImGui::Text("min=%.2f q1=%.2f med=%.2f q3=%.2f max=%.2f", mn, q1, med, q3, mx);
}

static void render_linear_regression(const SceneNode& n) {
    std::string xk = n.cols.count("x") ? n.cols.at("x") : "";
    std::string yk = n.cols.count("y") ? n.cols.at("y") : "";
    if (xk.empty() || yk.empty() || !n.series.count(xk) || !n.series.count(yk)) {
        ImGui::TextDisabled("linear_regression: faltan cols x/y");
        return;
    }
    const auto& xs = n.series.at(xk);
    const auto& ys = n.series.at(yk);
    int npts = std::min(xs.size(), ys.size());

    double slope, intercept;
    linear_regression(xs, ys, slope, intercept);

    if (ImPlot::BeginPlot(n.titulo.c_str(), ImVec2(-1, 280))) {
        ImPlot::SetupAxes(xk.c_str(), yk.c_str());
        ImPlot::PlotScatter("data", xs.data(), ys.data(), npts);
        std::vector<double> lx = xs, ly;
        ly.reserve(xs.size());
        for (double x : xs) ly.push_back(slope * x + intercept);
        ImPlot::PlotLine("fit", lx.data(), ly.data(), (int)lx.size());
        ImPlot::EndPlot();
    }
    ImGui::Text("y = %.4f * x + %.4f", slope, intercept);
}

static void render_node(const SceneNode& n) {
    if (n.tipo == "metric") { render_metric(n); return; }
    if (n.tipo == "text") { render_text(n); return; }
    if (n.tipo == "line_plot") { render_line_plot(n); return; }
    if (n.tipo == "bar_chart") { render_bar_chart(n); return; }
    if (n.tipo == "scatter") { render_scatter(n); return; }
    if (n.tipo == "heatmap") { render_heatmap(n); return; }
    if (n.tipo == "histogram") { render_histogram(n); return; }
    if (n.tipo == "box_plot") { render_box_plot(n); return; }
    if (n.tipo == "linear_regression") { render_linear_regression(n); return; }
    ImGui::TextDisabled("Tipo no soportado: %s", n.tipo.c_str());
}

static void render_scene_grid(const Scene& s) {
    int cols = std::max(1, s.layout.cols);
    int gap = s.layout.gap;

    float avail = ImGui::GetContentRegionAvail().x;
    float total_gap = gap * (cols - 1);
    float child_w = (avail - total_gap) / cols;

    for (size_t i = 0; i < s.nodes.size(); ++i) {
        const auto& n = s.nodes[i];
        int col = i % cols;
        if (col > 0) {
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x + col * (child_w + gap));
        }
        ImGui::BeginChild(("node_" + n.id + "_" + n.titulo).c_str(), ImVec2(child_w, 320), true);
        render_node(n);
        ImGui::EndChild();
    }
}

// ============ HTTP fetch ============

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

static std::string http_get(const std::string& host, int port, const std::string& path) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return "";
    struct hostent* he = gethostbyname(host.c_str());
    if (!he) { close(sock); return ""; }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return "";
    }

    std::string req = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
    send(sock, req.c_str(), req.size(), 0);

    std::string resp;
    char buf[4096];
    int n;
    while ((n = recv(sock, buf, sizeof(buf), 0)) > 0) {
        resp.append(buf, n);
    }
    close(sock);

    size_t pos = resp.find("\r\n\r\n");
    if (pos == std::string::npos) return "";
    return resp.substr(pos + 4);
}

// ============ Main ============

int main(int argc, char* argv[]) {
    std::string host = "localhost";
    int port = 8080;
    std::string scene_file;
    std::string screenshot_path;
    int width = 1400;
    int height = 900;
    int wait_frames = 5;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--host" && i + 1 < argc) host = argv[++i];
        else if (a == "--port" && i + 1 < argc) port = std::atoi(argv[++i]);
        else if (a == "--file" && i + 1 < argc) scene_file = argv[++i];
        else if (a == "--screenshot" && i + 1 < argc) screenshot_path = argv[++i];
        else if (a == "--width" && i + 1 < argc) width = std::atoi(argv[++i]);
        else if (a == "--height" && i + 1 < argc) height = std::atoi(argv[++i]);
        else if (a == "--wait" && i + 1 < argc) wait_frames = std::atoi(argv[++i]);
    }

    if (!glfwInit()) {
        std::fprintf(stderr, "Error: glfwInit fallo\n");
        return 1;
    }

#if defined(__APPLE__)
    const char* glsl_version = "#version 150";
#else
    const char* glsl_version = "#version 130";
#endif

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Si vamos a hacer screenshot sin mostrar ventana
    if (!screenshot_path.empty()) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }

    GLFWwindow* window = glfwCreateWindow(width, height, "Zeta Dashboard", NULL, NULL);
    if (!window) {
        std::fprintf(stderr, "Error: glfwCreateWindow fallo\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::fprintf(stderr, "Error: glewInit fallo\n");
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    Scene current_scene;
    bool first_load = true;
    double last_refresh = 0.0;

    // Cargar scene antes del loop (especialmente en modo screenshot)
    {
        std::string body;
        if (!scene_file.empty()) {
            FILE* f = fopen(scene_file.c_str(), "r");
            if (f) {
                char buf[8192];
                size_t n = fread(buf, 1, sizeof(buf) - 1, f);
                buf[n] = 0;
                body = std::string(buf, n);
                fclose(f);
            }
        } else if (!screenshot_path.empty()) {
            // Si no hay file pero hay screenshot, intentar HTTP
            body = http_get(host, port, "/api/grafo");
        }
        if (!body.empty()) {
            try {
                auto j = json::parse(body);
                current_scene = parse_scene(j);
            } catch (...) {}
        }
    }

    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Cargar scene: desde archivo o HTTP
        if (first_load || glfwGetTime() - last_refresh > 2.0) {
            std::string body;
            if (!scene_file.empty()) {
                FILE* f = fopen(scene_file.c_str(), "r");
                if (f) {
                    char buf[8192];
                    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
                    buf[n] = 0;
                    body = std::string(buf, n);
                    fclose(f);
                }
            } else {
                body = http_get(host, port, "/api/grafo");
            }
            if (!body.empty()) {
                try {
                    auto j = json::parse(body);
                    current_scene = parse_scene(j);
                    if (first_load) {
                        first_load = false;
                    }
                } catch (...) {}
            }
            last_refresh = glfwGetTime();
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Top bar
        ImGui::BeginMainMenuBar();
        char title_buf[256];
        snprintf(title_buf, sizeof(title_buf), "Zeta Dashboard  -  %s  (autor: %s, %zu nodos)",
                 current_scene.titulo.c_str(), current_scene.autor.c_str(), current_scene.nodes.size());
        ImGui::Text("%s", title_buf);
        ImGui::EndMainMenuBar();

        ImGuiIO& io_render = ImGui::GetIO();
        float menu_h = ImGui::GetFrameHeight();
        float panel_w = 220.0f;
        ImVec2 display = io_render.DisplaySize;
        ImGui::SetNextWindowPos(ImVec2(0, menu_h), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(display.x - panel_w, display.y - menu_h), ImGuiCond_Always);
        ImGui::Begin("Dashboard");
        if (current_scene.nodes.empty()) {
            ImGui::TextDisabled("Sin nodos. Ejecuta un script Zeta con scene() y agrega nodos.");
            ImGui::TextDisabled("O pasa --file <scene.json> para cargar un archivo.");
        } else {
            render_scene_grid(current_scene);
        }
        ImGui::End();

        // Settings panel
        ImGui::SetNextWindowPos(ImVec2(display.x - panel_w, menu_h), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(panel_w, display.y - menu_h), ImGuiCond_Always);
        ImGui::Begin("Layout");
        ImGui::Text("Layout: %s", current_scene.layout.tipo.c_str());
        ImGui::Text("Cols: %d  Gap: %d", current_scene.layout.cols, current_scene.layout.gap);
        ImGui::Text("Background: %s", current_scene.layout.background.c_str());
        if (!scene_file.empty()) {
            ImGui::Text("Source: %s", scene_file.c_str());
        } else {
            ImGui::Text("Source: http://%s:%d/api/grafo", host.c_str(), port);
        }
        if (ImGui::Button("Refresh")) last_refresh = 0.0;
        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        ImVec4 bg = hex_to_vec4(current_scene.layout.background, ImVec4(0.118f, 0.118f, 0.118f, 1.0f));
        glClearColor(bg.x, bg.y, bg.z, bg.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        frame++;
        if (!screenshot_path.empty() && frame >= wait_frames) {
            // Capturar y guardar PNG
            int fw, fh;
            glfwGetFramebufferSize(window, &fw, &fh);
            std::vector<unsigned char> pixels(fw * fh * 4);
            glReadPixels(0, 0, fw, fh, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

            // Voltear verticalmente (OpenGL origen es abajo-izquierda)
            std::vector<unsigned char> flipped(fw * fh * 4);
            for (int y = 0; y < fh; ++y) {
                memcpy(flipped.data() + y * fw * 4,
                       pixels.data() + (fh - 1 - y) * fw * 4,
                       fw * 4);
            }

            if (stbi_write_png(screenshot_path.c_str(), fw, fh, 4, flipped.data(), fw * 4)) {
                std::fprintf(stderr, "[OK] Screenshot guardado: %s (%dx%d)\n",
                             screenshot_path.c_str(), fw, fh);
            } else {
                std::fprintf(stderr, "[ERR] No se pudo guardar: %s\n", screenshot_path.c_str());
            }
            break;
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
