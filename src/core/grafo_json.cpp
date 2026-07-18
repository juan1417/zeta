#include "zeta/grafo_json.hpp"
#include "deps/json.hpp"
#include <fstream>
#include <charconv>

namespace zeta {

static void escape_json_string(std::ostream& os, const std::string& s) {
    os << '"';
    for (char c : s) {
        switch (c) {
            case '"': os << "\\\""; break;
            case '\\': os << "\\\\"; break;
            case '\n': os << "\\n"; break;
            case '\r': os << "\\r"; break;
            case '\t': os << "\\t"; break;
            default: os << c;
        }
    }
    os << '"';
}

static void write_val(std::ostream& os, double v) {
    if (es_null(v)) { os << "null"; return; }
    char buf[64];
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), v, std::chars_format::general, 15);
    if (ec == std::errc()) os.write(buf, ptr - buf);
    else os << "null";
}

static void write_node_json(std::ostream& os, const SceneNode& n, bool first) {
    if (!first) os << ',';
    os << '{';
    os << "\"tipo\":";
    escape_json_string(os, n.tipo);
    if (!n.id.empty()) {
        os << ",\"id\":";
        escape_json_string(os, n.id);
    }
    os << ",\"titulo\":";
    escape_json_string(os, n.titulo);
    if (!n.cols.empty()) {
        os << ",\"cols\":{";
        bool fc = true;
        for (const auto& [k, v] : n.cols) {
            if (!fc) os << ',';
            fc = false;
            escape_json_string(os, k);
            os << ':';
            escape_json_string(os, v);
        }
        os << '}';
    }
    if (!n.nums.empty()) {
        os << ",\"nums\":{";
        bool fc = true;
        for (const auto& [k, v] : n.nums) {
            if (!fc) os << ',';
            fc = false;
            escape_json_string(os, k);
            os << ':';
            write_val(os, v);
        }
        os << '}';
    }
    if (!n.strs.empty()) {
        os << ",\"strs\":{";
        bool fc = true;
        for (const auto& [k, v] : n.strs) {
            if (!fc) os << ',';
            fc = false;
            escape_json_string(os, k);
            os << ':';
            escape_json_string(os, v);
        }
        os << '}';
    }
    if (!n.series.empty()) {
        os << ",\"data\":{";
        bool fc = true;
        for (const auto& [k, v] : n.series) {
            if (!fc) os << ',';
            fc = false;
            escape_json_string(os, k);
            os << ':';
            os << '[';
            bool fv = true;
            for (double x : v) {
                if (!fv) os << ',';
                fv = false;
                write_val(os, x);
            }
            os << ']';
        }
        os << '}';
    }
    os << '}';
}

static void write_layout_json(std::ostream& os, const SceneLayout& l) {
    os << "\"layout\":{";
    os << "\"tipo\":"; escape_json_string(os, l.tipo);
    os << ",\"cols\":" << l.cols;
    os << ",\"gap\":" << l.gap;
    os << ",\"background\":"; escape_json_string(os, l.background);
    os << '}';
}

static SceneNode json_a_scene_node(const nlohmann::json& nj) {
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
                if (x.is_null()) v.push_back(crear_null());
                else if (x.is_number()) v.push_back(x.get<double>());
                else v.push_back(crear_null());
            }
            n.series[it.key()] = std::move(v);
        }
    }
    return n;
}

std::string guardar_grafo_json(const std::string& ruta, const SceneSpec& s) {
    std::ofstream file(ruta);
    if (!file.is_open()) return "Error: no se pudo abrir " + ruta;

    file << "{\"titulo\":";
    escape_json_string(file, s.titulo);
    file << ",\"autor\":";
    escape_json_string(file, s.autor);
    file << ",\"created_at\":" << s.created_at;
    file << ",\"updated_at\":" << s.updated_at;
    file << ',';
    write_layout_json(file, s.layout);
    file << ",\"nodes\":[";
    bool first = true;
    for (const auto& n : s.nodes) {
        write_node_json(file, n, first);
        first = false;
    }
    file << "]}";
    file.close();
    return "Guardado: " + ruta + " (" + std::to_string(s.nodes.size()) + " nodos)";
}

std::shared_ptr<SceneSpec> cargar_grafo_json(const std::string& ruta) {
    std::ifstream file(ruta);
    if (!file.is_open()) return nullptr;

    nlohmann::json j;
    try {
        j = nlohmann::json::parse(file);
    } catch (...) {
        return nullptr;
    }

    auto s = std::make_shared<SceneSpec>();
    s->titulo = j.value("titulo", "");
    s->autor = j.value("autor", "");
    s->created_at = j.value("created_at", 0.0);
    s->updated_at = j.value("updated_at", 0.0);

    if (j.contains("layout") && j["layout"].is_object()) {
        const auto& l = j["layout"];
        s->layout.tipo = l.value("tipo", "grid");
        s->layout.cols = l.value("cols", 2);
        s->layout.gap = l.value("gap", 10);
        s->layout.background = l.value("background", "#1e1e1e");
    }

    if (j.contains("nodes") && j["nodes"].is_array()) {
        for (const auto& nj : j["nodes"]) {
            s->nodes.push_back(json_a_scene_node(nj));
        }
    }

    return s;
}

} // namespace zeta
