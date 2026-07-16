#include "zeta/lexer.hpp"
#include "zeta/parser.hpp"
#include "zeta/interpreter.hpp"
#include "deps/crow_all.h"
#include <iostream>
#include <sstream>
#include <fstream>

using namespace zeta;

// Embedded HTML frontend
const std::string HTML_PAGE = R"HTML(<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Zeta Analytics</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        .chart-container { position: relative; height: 300px; }
        .metric-card { transition: transform 0.2s; }
        .metric-card:hover { transform: translateY(-2px); }
    </style>
</head>
<body class="bg-gray-100 min-h-screen">
    <nav class="bg-indigo-600 text-white p-4 shadow-lg">
        <div class="container mx-auto flex justify-between items-center">
            <h1 class="text-2xl font-bold">Zeta Analytics</h1>
            <div class="space-x-4">
                <button onclick="loadData()" class="bg-indigo-500 hover:bg-indigo-700 px-4 py-2 rounded">Datos</button>
                <button onclick="loadMetrics()" class="bg-indigo-500 hover:bg-indigo-700 px-4 py-2 rounded">Metricas</button>
                <button onclick="loadDashboard()" class="bg-indigo-500 hover:bg-indigo-700 px-4 py-2 rounded">Dashboard</button>
            </div>
        </div>
    </nav>

    <div class="container mx-auto p-6">
        <!-- Metrics Section -->
        <div id="metrics-section" class="mb-8">
            <h2 class="text-xl font-semibold mb-4 text-gray-700">Metricas KPI</h2>
            <div id="metrics" class="grid grid-cols-1 md:grid-cols-3 gap-4"></div>
        </div>

        <!-- Charts Section -->
        <div id="charts-section" class="mb-8">
            <h2 class="text-xl font-semibold mb-4 text-gray-700">Graficos</h2>
            <div id="charts" class="grid grid-cols-1 md:grid-cols-2 gap-6"></div>
        </div>

        <!-- Data Table Section -->
        <div id="data-section" class="mb-8">
            <h2 class="text-xl font-semibold mb-4 text-gray-700">Datos</h2>
            <div id="data-table" class="bg-white rounded-lg shadow overflow-hidden"></div>
        </div>

        <!-- Console Section -->
        <div class="mb-8">
            <h2 class="text-xl font-semibold mb-4 text-gray-700">Consola Zeta</h2>
            <div class="bg-gray-900 rounded-lg p-4">
                <textarea id="zeta-code" class="w-full h-40 bg-gray-800 text-green-400 font-mono p-3 rounded resize-none" placeholder="# Escribe codigo Zeta aqui...
$datos = load_csv('datos.csv')
print(mean($datos:valor))"></textarea>
                <button onclick="runCode()" class="mt-3 bg-green-600 hover:bg-green-700 text-white px-6 py-2 rounded font-semibold">Ejecutar</button>
                <pre id="console-output" class="mt-3 text-green-400 font-mono text-sm whitespace-pre-wrap"></pre>
            </div>
        </div>
    </div>

    <script>
        let charts = {};

        async function loadData() {
            try {
                const res = await fetch('/api/datos');
                const data = await res.json();
                renderDataTable(data);
            } catch (e) {
                document.getElementById('data-table').innerHTML = '<p class="text-red-500 p-4">Error cargando datos</p>';
            }
        }

        async function loadMetrics() {
            try {
                const res = await fetch('/api/metricas');
                const metrics = await res.json();
                renderMetrics(metrics);
            } catch (e) {
                document.getElementById('metrics').innerHTML = '<p class="text-red-500 p-4">Error cargando metricas</p>';
            }
        }

        async function loadDashboard() {
            try {
                const [dataRes, metricsRes] = await Promise.all([
                    fetch('/api/datos'),
                    fetch('/api/metricas')
                ]);
                const data = await dataRes.json();
                const metrics = await metricsRes.json();
                renderMetrics(metrics);
                renderCharts(data);
            } catch (e) {
                console.error('Error loading dashboard:', e);
            }
        }

        async function runCode() {
            const code = document.getElementById('zeta-code').value;
            const output = document.getElementById('console-output');
            output.textContent = 'Ejecutando...';
            try {
                const res = await fetch('/api/run', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ code })
                });
                const result = await res.json();
                if (result.error) {
                    output.textContent = 'Error: ' + result.error;
                } else {
                    output.textContent = result.output || '(sin salida)';
                    // Auto-refresh dashboard after running code
                    loadDashboard();
                }
            } catch (e) {
                output.textContent = 'Error de conexion: ' + e.message;
            }
        }

        function renderMetrics(metrics) {
            const container = document.getElementById('metrics');
            if (!metrics || metrics.length === 0) {
                container.innerHTML = '<p class="text-gray-500">No hay metricas registradas</p>';
                return;
            }
            container.innerHTML = metrics.map(m => `
                <div class="metric-card bg-white rounded-lg shadow p-6">
                    <h3 class="text-sm font-medium text-gray-500">${m.nombre}</h3>
                    <p class="text-3xl font-bold text-indigo-600 mt-2">${m.valor}</p>
                </div>
            `).join('');
        }

        function renderCharts(data) {
            const container = document.getElementById('charts');
            container.innerHTML = '';
            charts = {};

            if (!data || typeof data !== 'object') return;

            Object.entries(data).forEach(([col, values], idx) => {
                if (!Array.isArray(values) || values.length === 0) return;
                const numericValues = values.filter(v => v !== null && typeof v === 'number');
                if (numericValues.length === 0) return;

                const chartId = 'chart-' + idx;
                container.innerHTML += `
                    <div class="bg-white rounded-lg shadow p-4">
                        <h3 class="font-semibold text-gray-700 mb-2">${col}</h3>
                        <div class="chart-container"><canvas id="${chartId}"></canvas></div>
                    </div>
                `;

                setTimeout(() => {
                    const ctx = document.getElementById(chartId);
                    if (!ctx) return;
                    charts[col] = new Chart(ctx, {
                        type: 'line',
                        data: {
                            labels: numericValues.map((_, i) => i + 1),
                            datasets: [{
                                label: col,
                                data: numericValues,
                                borderColor: `hsl(${idx * 60}, 70%, 50%)`,
                                backgroundColor: `hsla(${idx * 60}, 70%, 50%, 0.1)`,
                                fill: true,
                                tension: 0.3
                            }]
                        },
                        options: { responsive: true, maintainAspectRatio: false }
                    });
                }, 100);
            });
        }

        function renderDataTable(data) {
            const container = document.getElementById('data-table');
            if (!data || typeof data !== 'object') {
                container.innerHTML = '<p class="p-4 text-gray-500">Sin datos</p>';
                return;
            }

            const columns = Object.keys(data);
            if (columns.length === 0) {
                container.innerHTML = '<p class="p-4 text-gray-500">Sin datos</p>';
                return;
            }

            const rowCount = Math.max(...columns.map(c => data[c].length));

            let html = '<table class="w-full text-sm"><thead><tr class="bg-gray-50">';
            columns.forEach(col => {
                html += `<th class="px-4 py-3 text-left font-medium text-gray-500">${col}</th>`;
            });
            html += '</tr></thead><tbody>';

            for (let i = 0; i < Math.min(rowCount, 50); i++) {
                html += '<tr class="border-t">';
                columns.forEach(col => {
                    const val = data[col][i];
                    const display = val === null ? '<span class="text-gray-400">null</span>' : val;
                    html += `<td class="px-4 py-2">${display}</td>`;
                });
                html += '</tr>';
            }

            if (rowCount > 50) {
                html += `<tr><td colspan="${columns.length}" class="px-4 py-2 text-gray-500 text-center">... y ${rowCount - 50} filas mas</td></tr>`;
            }

            html += '</tbody></table>';
            container.innerHTML = html;
        }

        // Load dashboard on page load
        window.addEventListener('load', loadDashboard);
    </script>
</body>
</html>)HTML";

int main(int argc, char* argv[]) {
    int port = 8080;
    std::string script_file;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--script" && i + 1 < argc) {
            script_file = argv[++i];
        } else if (arg == "--help") {
            std::cout << "Uso: zeta_server [--port PORT] [--script ARCHIVO]\n";
            return 0;
        }
    }

    // Create shared interpreter instance
    auto interpreter = std::make_shared<Interpreter>();

    // Load initial script if provided
    if (!script_file.empty()) {
        interpreter->set_script_path(script_file);
        std::ifstream file(script_file);
        if (!file.is_open()) {
            std::cerr << "Error: No se pudo abrir " << script_file << "\n";
            return 1;
        }
        std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        try {
            Lexer lexer(code);
            auto tokens = lexer.tokenizar();
            Parser parser(std::move(tokens));
            auto ast = parser.parsear();
            interpreter->ejecutar(std::move(ast));
            std::cout << "Script cargado: " << script_file << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Error ejecutando script: " << e.what() << "\n";
        }
    }

    crow::SimpleApp app;

    // Serve HTML frontend
    CROW_ROUTE(app, "/")
    ([&](const crow::request&, crow::response& res) {
        res.set_header("Content-Type", "text/html; charset=utf-8");
        res.write(HTML_PAGE);
        res.end();
    });

    // API: Get all data as JSON
    CROW_ROUTE(app, "/api/datos")
    ([&](const crow::request&, crow::response& res) {
        auto json = interpreter->obtener_datos_json();
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        res.write(json.dump(2));
        res.end();
    });

    // API: Get metrics
    CROW_ROUTE(app, "/api/metricas")
    ([&](const crow::request&, crow::response& res) {
        auto metrics = interpreter->obtener_metricas_json();
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& m : metrics) arr.push_back(m);
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        res.write(arr.dump(2));
        res.end();
    });

    // API: Get dashboard config
    CROW_ROUTE(app, "/api/dashboard")
    ([&](const crow::request&, crow::response& res) {
        auto json = interpreter->obtener_dashboard_json();
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        if (json.is_null()) {
            res.write(R"({"titulo":"","elementos":[]})");
        } else {
            res.write(json.dump(2));
        }
        res.end();
    });

    // API: Get scene spec (graph)
    CROW_ROUTE(app, "/api/grafo")
    ([&](const crow::request&, crow::response& res) {
        auto json = interpreter->obtener_grafo_json();
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        if (json.is_null() || json.empty()) {
            res.write(R"({"titulo":"","autor":"","layout":{"tipo":"grid","cols":2,"gap":10,"background":"#1e1e1e"},"nodes":[]})");
        } else {
            res.write(json.dump(2));
        }
        res.end();
    });

    // API: Run Zeta code
    CROW_ROUTE(app, "/api/run").methods("POST"_method)
    ([&](const crow::request& req, crow::response& res) {
        auto body = nlohmann::json::parse(req.body, nullptr, false);
        if (body.is_discarded() || !body.contains("code")) {
            res.code = 400;
            res.write(R"({"error":"Body debe contener 'code'"})");
            res.end();
            return;
        }

        std::string code = body["code"].get<std::string>();
        std::ostringstream output;
        auto old_buf = std::cout.rdbuf(output.rdbuf());

        try {
            Lexer lexer(code);
            auto tokens = lexer.tokenizar();
            Parser parser(std::move(tokens));
            auto ast = parser.parsear();
            auto result = interpreter->ejecutar(std::move(ast));
            if (result && result->tipo == ValorImpl::ERR) {
                std::cout.rdbuf(old_buf);
                res.code = 500;
                res.write(R"({"error":")" + result->err_val.mensaje + R"("})");
                res.end();
                return;
            }
        } catch (const std::exception& e) {
            std::cout.rdbuf(old_buf);
            res.code = 500;
            res.write(R"({"error":")" + std::string(e.what()) + R"("})");
            res.end();
            return;
        }

        std::cout.rdbuf(old_buf);
        res.set_header("Content-Type", "application/json");
        nlohmann::json response;
        response["output"] = output.str();
        response["datos"] = interpreter->obtener_datos_json();
        response["metricas"] = interpreter->obtener_metricas_json();
        res.write(response.dump(2));
        res.end();
    });

    std::cout << "\n=== Zeta Server ===\n";
    std::cout << "Puerto: " << port << "\n";
    std::cout << "URL: http://localhost:" << port << "\n";
    std::cout << "API:  http://localhost:" << port << "/api/datos\n";
    std::cout << "==================\n\n";

    // Auto-register user routes: after first /api/run, interpreter may have
    // rutas_registradas_ populated. We register them on the fly the first time
    // /api/run completes. (Routes added to crow must be done before app.run().)
    // Workaround: re-bind CROW_ROUTE at app.run() time by checking inside
    // a "shim" route /api/run that also installs user routes.
    // The simpler approach: register user routes on the next /api/run call.
    // But since crow requires routes before run(), we use a global set + a
    // CROW_ROUTE for each method/path that's pre-registered but the handler
    // checks a map.
    //
    // For simplicity and to keep crow happy: we provide a single wildcard
    // CROW_ROUTE that dispatches based on the interpreter's registered routes.
    // But crow's routing is static, so we use the CROW_CATCHALL_ROUTE pattern.
    //
    // We expose user routes via a "/u/*" prefix and translate internally.
    // GET/POST/PUT/DELETE/PATCH /u/<path> -> lookup path in interpreter->obtener_rutas().
    // Crow doesn't have a catch-all method, so we register the same path
    // for each HTTP method.
    auto user_route_handler = [&](const crow::request& req, crow::response& res, std::string path) {
        res.set_header("Access-Control-Allow-Origin", "*");

        std::string full_path = "/" + path;
        std::string metodo = crow::method_name(req.method);

        // Search interpreter's registered routes for a match
        const auto& rutas = interpreter->obtener_rutas();
        for (const auto& r : rutas) {
            if (r.path == full_path && r.metodo == metodo) {
                // Build request dict for the handler
                nlohmann::json req_dict;
                req_dict["method"] = metodo;
                req_dict["path"] = full_path;
                req_dict["body"] = req.body;
                req_dict["query"] = nlohmann::json::object();
                req_dict["headers"] = nlohmann::json::object();
                for (const auto& key : req.url_params.keys()) {
                    char* v = req.url_params.get(key);
                    if (v) req_dict["query"][key] = std::string(v);
                }
                for (const auto& h : req.headers) {
                    req_dict["headers"][h.first] = h.second;
                }

                ValorZeta arg = mk_str(req_dict.dump());
                std::vector<ValorZeta> args = {arg};
                ValorZeta result;
                try {
                    result = interpreter->llamar_usuario_directo(r.handler, args);
                } catch (const std::exception& e) {
                    res.code = 500;
                    res.write(std::string(R"({"error":")") + e.what() + R"("})");
                    res.end();
                    return;
                }

                if (result && result->tipo == ValorImpl::ERR) {
                    res.code = 500;
                    nlohmann::json j;
                    j["error"] = result->err_val.mensaje;
                    res.write(j.dump());
                    res.end();
                    return;
                }

                nlohmann::json out_body;
                int status = 200;
                if (result && result->tipo == ValorImpl::DICT) {
                    auto& m = result->dict_val;
                    if (m.count("status")) {
                        auto& sv = m["status"];
                        if (sv && sv->tipo == ValorImpl::NUM) {
                            status = static_cast<int>(sv->num_val);
                        }
                    }
                    if (m.count("body")) {
                        auto& bv = m["body"];
                        if (bv && bv->tipo == ValorImpl::STR) {
                            out_body = bv->str_val;
                        } else if (bv && bv->tipo == ValorImpl::NUM) {
                            out_body = bv->num_val;
                        } else {
                            out_body = nullptr;
                        }
                    }
                    if (m.count("headers")) {
                        auto& hv = m["headers"];
                        if (hv && hv->tipo == ValorImpl::DICT) {
                            for (const auto& [k, v] : hv->dict_val) {
                                if (v && v->tipo == ValorImpl::STR) {
                                    res.set_header(k, v->str_val);
                                }
                            }
                        }
                    }
                } else if (result && result->tipo == ValorImpl::STR) {
                    out_body = result->str_val;
                } else if (result && result->tipo == ValorImpl::NUM) {
                    out_body = result->num_val;
                } else {
                    out_body = nullptr;
                }

                res.code = status;
                res.set_header("Content-Type", "application/json");
                res.write(out_body.dump());
                res.end();
                return;
            }
        }

        res.code = 404;
        nlohmann::json j;
        j["error"] = "No hay ruta registrada para " + metodo + " " + full_path;
        res.write(j.dump());
        res.end();
    };

    CROW_ROUTE(app, "/u/<path>").methods(crow::HTTPMethod::Get)(user_route_handler);
    CROW_ROUTE(app, "/u/<path>").methods(crow::HTTPMethod::Post)(user_route_handler);
    CROW_ROUTE(app, "/u/<path>").methods(crow::HTTPMethod::Put)(user_route_handler);
    CROW_ROUTE(app, "/u/<path>").methods(crow::HTTPMethod::Delete)(user_route_handler);
    CROW_ROUTE(app, "/u/<path>").methods(crow::HTTPMethod::Patch)(user_route_handler);

    app.port(port).multithreaded().run();

    return 0;
}
