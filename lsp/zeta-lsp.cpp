#include "transport.hpp"
#include "protocol.hpp"
#include "analyzer.hpp"
#include <iostream>
#include <string>

using json = nlohmann::json;
using namespace zeta_lsp;

static bool running = true;
static Analyzer analyzer;
static Transport transport;

json handle_initialize(const json& params) {
    json caps = {
        {"textDocumentSync", 1},
        {"hoverProvider", true},
        {"definitionProvider", true},
        {"documentSymbolProvider", true},
        {"referencesProvider", true},
        {"completionProvider", {
            {"triggerCharacters", {".", "(", ":"}},
            {"resolveProvider", false}
        }},
        {"signatureHelpProvider", {
            {"triggerCharacters", {"("}}
        }}
    };
    return {
        {"capabilities", caps},
        {"serverInfo", {{"name", "zeta-lsp"}, {"version", "0.1.0"}}}
    };
}

json handle_completion(const json& params) {
    auto pos_params = TextDocumentPosition::from_json(params);
    auto items = analyzer.get_completions(pos_params.textDocument.uri, pos_params.position);
    json result_items = json::array();
    for (auto& item : items) result_items.push_back(item.to_json());
    return {{"isIncomplete", false}, {"items", result_items}};
}

json handle_hover(const json& params) {
    auto pos_params = TextDocumentPosition::from_json(params);
    auto hover = analyzer.get_hover(pos_params.textDocument.uri, pos_params.position);
    return hover ? hover->to_json() : json(nullptr);
}

json handle_definition(const json& params) {
    auto pos_params = TextDocumentPosition::from_json(params);
    auto def = analyzer.get_definition(pos_params.textDocument.uri, pos_params.position);
    if (!def) return json(nullptr);
    return def->to_json();
}

json handle_document_symbol(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    auto symbols = analyzer.get_document_symbols(uri);
    json result = json::array();
    for (auto& s : symbols) result.push_back(s.to_json());
    return result;
}

json handle_signature_help(const json& params) {
    auto pos_params = TextDocumentPosition::from_json(params);
    auto help = analyzer.get_signature_help(pos_params.textDocument.uri, pos_params.position);
    return help ? help->to_json() : json(nullptr);
}

json handle_references(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    Position pos = Position::from_json(params["position"]);
    auto refs = analyzer.get_references(uri, pos);
    json result = json::array();
    for (auto& r : refs) result.push_back(r.to_json());
    return result;
}

void handle_diagnostics(const std::string& uri) {
    auto diags = analyzer.get_diagnostics(uri);
    json params = DiagnosticParams{uri, diags}.to_json();
    transport.write_message({{"jsonrpc", "2.0"}, {"method", "textDocument/publishDiagnostics"}, {"params", params}});
}

int main() {
    std::cerr << "[zeta-lsp] Starting on stdio..." << std::endl;

    while (running) {
        try {
            json msg = transport.read_message();

            if (msg.contains("method")) {
                std::string method = msg["method"];
                json params = msg.value("params", json::object());

                if (method == "initialize") {
                    auto result = handle_initialize(params);
                    transport.write_message({{"jsonrpc", "2.0"}, {"id", msg["id"]}, {"result", result}});
                }
                else if (method == "initialized") {
                    // no-op
                }
                else if (method == "shutdown") {
                    transport.write_message({{"jsonrpc", "2.0"}, {"id", msg["id"]}, {"result", json(nullptr)}});
                }
                else if (method == "exit") {
                    running = false;
                }
                else if (method == "textDocument/didOpen") {
                    std::string uri = params["textDocument"]["uri"];
                    std::string text = params["textDocument"]["text"];
                    analyzer.analyze(uri, text);
                    handle_diagnostics(uri);
                }
                else if (method == "textDocument/didChange") {
                    std::string uri = params["textDocument"]["uri"];
                    if (!params["contentChanges"].empty()) {
                        std::string text = params["contentChanges"][0]["text"];
                        analyzer.analyze(uri, text);
                    }
                    handle_diagnostics(uri);
                }
                else if (method == "textDocument/didSave") {
                    std::string uri = params["textDocument"]["uri"];
                    handle_diagnostics(uri);
                }
                else if (method == "textDocument/completion") {
                    auto result = handle_completion(params);
                    transport.write_message({{"jsonrpc", "2.0"}, {"id", msg["id"]}, {"result", result}});
                }
                else if (method == "textDocument/hover") {
                    auto result = handle_hover(params);
                    transport.write_message({{"jsonrpc", "2.0"}, {"id", msg["id"]}, {"result", result}});
                }
                else if (method == "textDocument/definition") {
                    auto result = handle_definition(params);
                    transport.write_message({{"jsonrpc", "2.0"}, {"id", msg["id"]}, {"result", result}});
                }
                else if (method == "textDocument/documentSymbol") {
                    auto result = handle_document_symbol(params);
                    transport.write_message({{"jsonrpc", "2.0"}, {"id", msg["id"]}, {"result", result}});
                }
                else if (method == "textDocument/signatureHelp") {
                    auto result = handle_signature_help(params);
                    transport.write_message({{"jsonrpc", "2.0"}, {"id", msg["id"]}, {"result", result}});
                }
                else if (method == "textDocument/references") {
                    auto result = handle_references(params);
                    transport.write_message({{"jsonrpc", "2.0"}, {"id", msg["id"]}, {"result", result}});
                }
                else {
                    if (msg.contains("id")) {
                        transport.write_message({
                            {"jsonrpc", "2.0"}, {"id", msg["id"]},
                            {"error", {{"code", -32601}, {"message", "Method not found: " + method}}}
                        });
                    }
                }
            }
        } catch (const std::exception& e) {
            if (std::string(e.what()) == "EOF" || std::string(e.what()) == "Incomplete body") {
                break;
            }
            std::cerr << "[zeta-lsp] Error: " << e.what() << std::endl;
        }
    }

    std::cerr << "[zeta-lsp] Shutting down." << std::endl;
    return 0;
}
