#include "analyzer.hpp"
#include "../include/zeta/valor_zeta.hpp"
#include <algorithm>

namespace zeta_lsp {

Analyzer::Analyzer() {}

void Analyzer::analyze(const std::string& uri, const std::string& source) {
    DocumentState& doc = documents_[uri];
    doc.source = source;
    doc.diagnostics.clear();
    doc.symbols.clear();

    try {
        Lexer lexer(source);
        doc.tokens = lexer.tokenizar();
    } catch (const std::exception& e) {
        add_diagnostic(doc.diagnostics, 0, 0, 0, 1, std::string("Lexer error: ") + e.what());
        doc.ast = nullptr;
        return;
    }

    try {
        Parser parser(doc.tokens);
        doc.ast = parser.parsear();
    } catch (const std::exception& e) {
        std::string msg = e.what();
        int line = 0, col = 0;
        auto pos = msg.find("linea ");
        if (pos != std::string::npos) {
            line = std::stoi(msg.substr(pos + 6)) - 1;
        }
        add_diagnostic(doc.diagnostics, line, col, col + 20, 1, msg);
        doc.ast = nullptr;
        return;
    }

    if (doc.ast) {
        walk_ast(doc.ast.get(), uri, doc.symbols, doc.diagnostics);
    }
}

void Analyzer::remove(const std::string& uri) {
    documents_.erase(uri);
}

void Analyzer::walk_ast(const NodoAST* node, const std::string& uri,
                         std::vector<Symbol>& symbols, std::vector<Diagnostic>& diagnostics,
                         int depth) {
    if (!node) return;

    int line = node->linea > 0 ? node->linea - 1 : 0;
    int col = node->columna > 0 ? node->columna - 1 : 0;

    switch (node->tipo) {
        case TipoNodoAST::DECLARACION_FN: {
            std::string fname = "$" + node->valor_texto;
            std::string doc = "fn " + fname + "(";
            for (size_t i = 0; i < node->parametros.size(); i++) {
                if (i > 0) doc += ", ";
                doc += "$" + node->parametros[i];
            }
            doc += ")";
            add_symbol(symbols, fname, SymbolKind::Function, uri, line, col,
                       "function", node->parametros, doc);
            break;
        }

        case TipoNodoAST::DECLARACION_CLASE: {
            std::string cname = node->valor_texto;
            add_symbol(symbols, cname, SymbolKind::Class, uri, line, col, "class");
            break;
        }

        case TipoNodoAST::ASIGNACION: {
            std::string vname;
            if (node->valor_texto.size() > 6 && node->valor_texto.substr(0, 6) == "FIELD:") {
                vname = node->valor_texto.substr(6);
            } else {
                vname = node->valor_texto;
            }
            if (!vname.empty() && vname[0] != '$') vname = "$" + vname;

            std::string type_hint;
            if (!node->hijos.empty()) {
                type_hint = token_type_name(node->hijos[0]->tipo);
            }
            add_symbol(symbols, vname, SymbolKind::Variable, uri, line, col, type_hint);
            break;
        }

        case TipoNodoAST::FOR_IN: {
            std::string vname = "$" + node->valor_texto;
            add_symbol(symbols, vname, SymbolKind::Variable, uri, line, col, "any");
            break;
        }

        case TipoNodoAST::VARIABLE: {
            std::string vname = "$" + node->valor_texto;
            bool found = false;
            for (auto& s : symbols) {
                if (s.name == vname && s.kind != SymbolKind::Function) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                bool is_builtin = builtins_.find(node->valor_texto) != nullptr;
                if (!is_builtin) {
                    add_diagnostic(diagnostics, line, col, col + (int)vname.size(),
                                   2, "Undefined variable: " + vname);
                }
            }
            break;
        }

        case TipoNodoAST::LLAMADA_FUNCION: {
            std::string fname = node->nombre_funcion;
            auto* builtin = builtins_.find(fname);
            if (builtin) {
                std::string doc = "## " + fname + "\n\n" + builtin->description + "\n\n```zeta\n";
                doc += fname + "(";
                for (size_t i = 0; i < builtin->params.size(); i++) {
                    if (i > 0) doc += ", ";
                    doc += "$" + builtin->params[i].name;
                }
                doc += ") -> " + builtin->returnType + "\n```\n";
                if (!builtin->example.empty()) {
                    doc += "\n**Example:**\n```zeta\n" + builtin->example + "\n```";
                }
                add_symbol(symbols, fname, SymbolKind::Function, uri, line, col,
                           builtin->returnType, {}, doc);
            }
            break;
        }

        default:
            break;
    }

    for (auto& child : node->hijos) {
        walk_ast(child.get(), uri, symbols, diagnostics, depth + 1);
    }
}

void Analyzer::add_symbol(std::vector<Symbol>& symbols, const std::string& name,
                           SymbolKind kind, const std::string& uri, int line, int col,
                           const std::string& type_hint,
                           const std::vector<std::string>& params,
                           const std::string& doc) {
    for (auto& s : symbols) {
        if (s.name == name && s.kind == kind) {
            s.definition = {uri, {Position{line, col}, Position{line, col + (int)name.size()}}};
            if (!type_hint.empty()) s.type_hint = type_hint;
            return;
        }
    }
    symbols.push_back({name, kind, {uri, {Position{line, col}, Position{line, col + (int)name.size()}}},
                       type_hint, params, doc});
}

void Analyzer::add_diagnostic(std::vector<Diagnostic>& diagnostics,
                               int line, int col, int end_col,
                               int severity, const std::string& message) {
    diagnostics.push_back({
        {Position{line, col}, Position{line, end_col}},
        severity, message
    });
}

bool Analyzer::position_in_token(const Position& pos, const Token& tok) const {
    int tok_line = tok.linea - 1;
    int tok_col = tok.columna - 1;
    int tok_end = tok_col + (int)tok.lexema.size();
    return pos.line == tok_line && pos.character >= tok_col && pos.character < tok_end;
}

const Token* Analyzer::token_at_position(const std::string& uri, const Position& pos) const {
    auto it = documents_.find(uri);
    if (it == documents_.end()) return nullptr;
    for (auto& tok : it->second.tokens) {
        if (position_in_token(pos, tok)) return &tok;
    }
    return nullptr;
}

const Symbol* Analyzer::find_symbol_at(const std::string& uri, const Position& pos) const {
    auto it = documents_.find(uri);
    if (it == documents_.end()) return nullptr;
    for (auto& s : it->second.symbols) {
        auto& r = s.definition.range;
        if (pos.line >= r.start.line && pos.line <= r.end.line &&
            pos.character >= r.start.character && pos.character < r.end.character) {
            return &s;
        }
    }
    return nullptr;
}

std::string Analyzer::token_type_name(TipoNodoAST tipo) const {
    switch (tipo) {
        case TipoNodoAST::LITERAL_NUMERO: return "number";
        case TipoNodoAST::LITERAL_CADENA: return "string";
        case TipoNodoAST::LITERAL_BOOL: return "bool";
        case TipoNodoAST::LITERAL_NULL: return "null";
        case TipoNodoAST::VECTOR: return "vector";
        case TipoNodoAST::MATRIZ: return "matrix";
        case TipoNodoAST::DICCIONARIO: return "dict";
        case TipoNodoAST::LLAMADA_FUNCION: return "function";
        default: return "any";
    }
}

std::vector<CompletionItem> Analyzer::get_completions(const std::string& uri, const Position& pos) {
    std::vector<CompletionItem> items;

    auto it = documents_.find(uri);
    if (it != documents_.end()) {
        for (auto& s : it->second.symbols) {
            CompletionItem item;
            item.label = s.name;
            item.detail = s.type_hint;
            if (s.kind == SymbolKind::Function) {
                item.kind = CompletionItemKind::Function;
                std::string sig = s.name + "(";
                for (size_t i = 0; i < s.params.size(); i++) {
                    if (i > 0) sig += ", ";
                    sig += "$" + s.params[i];
                }
                sig += ")";
                item.detail = sig;
            } else if (s.kind == SymbolKind::Class) {
                item.kind = CompletionItemKind::Class;
            } else {
                item.kind = CompletionItemKind::Variable;
            }
            if (!s.documentation.empty()) {
                item.documentation = MarkupContent{"markdown", s.documentation};
            }
            items.push_back(item);
        }
    }

    for (auto& b : builtins_.all()) {
        CompletionItem item;
        item.label = b.name;
        item.kind = CompletionItemKind::Function;
        std::string sig = b.name + "(";
        for (size_t i = 0; i < b.params.size(); i++) {
            if (i > 0) sig += ", ";
            sig += "$" + b.params[i].name;
        }
        sig += ") -> " + b.returnType;
        item.detail = sig;
        std::string doc = b.description;
        if (!b.example.empty()) doc += "\n\nExample: " + b.example;
        item.documentation = MarkupContent{"markdown", doc};
        items.push_back(item);
    }

    std::sort(items.begin(), items.end(), [](const CompletionItem& a, const CompletionItem& b) {
        return a.label < b.label;
    });
    items.erase(std::unique(items.begin(), items.end(),
        [](const CompletionItem& a, const CompletionItem& b) { return a.label == b.label; }),
        items.end());

    return items;
}

std::optional<Hover> Analyzer::get_hover(const std::string& uri, const Position& pos) {
    auto* sym = find_symbol_at(uri, pos);
    if (sym) {
        std::string md = "**" + sym->name + "**";
        if (!sym->type_hint.empty()) md += " : " + sym->type_hint;
        if (!sym->documentation.empty()) md += "\n\n" + sym->documentation;
        return Hover{{"markdown", md}, sym->definition.range};
    }

    auto* tok = token_at_position(uri, pos);
    if (tok) {
        if (tok->tipo == zeta::TipoToken::NUMERO) {
            return Hover{{"markdown", "**number** `" + tok->lexema + "`"}, std::nullopt};
        }
        if (tok->tipo == zeta::TipoToken::CADENA) {
            return Hover{{"markdown", "**string** `" + tok->lexema + "`"}, std::nullopt};
        }
        if (tok->tipo == zeta::TipoToken::TRUE_LITERAL || tok->tipo == zeta::TipoToken::FALSE_LITERAL) {
            return Hover{{"markdown", "**bool** `" + tok->lexema + "`"}, std::nullopt};
        }
        if (tok->tipo == zeta::TipoToken::NULL_LITERAL) {
            return Hover{{"markdown", "**null** (NaN internally)"}, std::nullopt};
        }

        auto* builtin = builtins_.find(tok->lexema);
        if (builtin) {
            std::string md = "## " + builtin->name + "\n\n" + builtin->description + "\n\n```zeta\n";
            md += builtin->name + "(";
            for (size_t i = 0; i < builtin->params.size(); i++) {
                if (i > 0) md += ", ";
                md += "$" + builtin->params[i].name + ": " + builtin->params[i].type;
            }
            md += ") -> " + builtin->returnType + "\n```\n";
            if (!builtin->example.empty()) {
                md += "\n**Example:**\n```zeta\n" + builtin->example + "\n```";
            }
            return Hover{{"markdown", md}, std::nullopt};
        }
    }

    return std::nullopt;
}

std::optional<Location> Analyzer::get_definition(const std::string& uri, const Position& pos) {
    auto* sym = find_symbol_at(uri, pos);
    if (sym) return sym->definition;

    auto* tok = token_at_position(uri, pos);
    if (tok && tok->tipo == zeta::TipoToken::IDENTIFICADOR) {
        auto it = documents_.find(uri);
        if (it != documents_.end()) {
            for (auto& s : it->second.symbols) {
                if (s.name == "$" + tok->lexema) {
                    return s.definition;
                }
            }
        }
    }

    return std::nullopt;
}

std::vector<DocumentSymbol> Analyzer::get_document_symbols(const std::string& uri) {
    std::vector<DocumentSymbol> result;
    auto it = documents_.find(uri);
    if (it == documents_.end()) return result;

    for (auto& s : it->second.symbols) {
        DocumentSymbol ds;
        ds.name = s.name;
        ds.kind = s.kind;
        ds.range = s.definition.range;
        ds.selectionRange = s.definition.range;
        if (!s.type_hint.empty()) ds.detail = s.type_hint;
        result.push_back(ds);
    }
    return result;
}

std::optional<SignatureHelp> Analyzer::get_signature_help(const std::string& uri, const Position& pos) {
    auto it = documents_.find(uri);
    if (it == documents_.end()) return std::nullopt;

    int target_line = pos.line;

    for (int i = (int)it->second.tokens.size() - 1; i >= 0; i--) {
        auto& tok = it->second.tokens[i];
        if (tok.tipo == zeta::TipoToken::PARENTESIS_ABRE && tok.linea - 1 == target_line) {
            int func_col = tok.columna - 2;
            if (func_col < 0) continue;

            std::string func_name;
            for (auto& t : it->second.tokens) {
                if (t.linea - 1 == target_line && t.columna - 1 == func_col - (int)t.lexema.size() + 1) {
                    func_name = t.lexema;
                    break;
                }
            }

            if (!func_name.empty()) {
                auto* builtin = builtins_.find(func_name);
                if (builtin) {
                    SignatureInformation sig;
                    sig.label = builtin->name + "(";
                    for (size_t j = 0; j < builtin->params.size(); j++) {
                        if (j > 0) sig.label += ", ";
                        sig.label += "$" + builtin->params[j].name;
                    }
                    sig.label += ") -> " + builtin->returnType;
                    sig.documentation = MarkupContent{"markdown", builtin->description};
                    for (auto& p : builtin->params) {
                        sig.parameters.push_back({"$" + p.name + ": " + p.type,
                                                  MarkupContent{"markdown", p.name}});
                    }
                    return SignatureHelp{{sig}, 0, 0};
                }
            }
        }
    }

    return std::nullopt;
}

std::vector<Location> Analyzer::get_references(const std::string& uri, const Position& pos) {
    std::vector<Location> refs;
    auto* sym = find_symbol_at(uri, pos);
    if (!sym) return refs;

    auto it = documents_.find(uri);
    if (it == documents_.end()) return refs;

    for (auto& s : it->second.symbols) {
        if (s.name == sym->name) {
            refs.push_back(s.definition);
        }
    }
    return refs;
}

std::vector<Diagnostic> Analyzer::get_diagnostics(const std::string& uri) {
    auto it = documents_.find(uri);
    if (it == documents_.end()) return {};
    return it->second.diagnostics;
}

const std::string& Analyzer::get_source(const std::string& uri) const {
    auto it = documents_.find(uri);
    static const std::string empty;
    return it != documents_.end() ? it->second.source : empty;
}

} // namespace zeta_lsp
