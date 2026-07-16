#pragma once
#include "protocol.hpp"
#include "builtins.hpp"
#include "../include/zeta/parser.hpp"
#include "../include/zeta/lexer.hpp"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace zeta_lsp {

using zeta::Token;
using zeta::NodoAST;
using zeta::TipoNodoAST;
using zeta::Lexer;
using zeta::Parser;

struct Symbol {
    std::string name;
    SymbolKind kind;
    Location definition;
    std::string type_hint;
    std::vector<std::string> params;
    std::string documentation;
};

class Analyzer {
public:
    Analyzer();

    void analyze(const std::string& uri, const std::string& source);
    void remove(const std::string& uri);

    std::vector<CompletionItem> get_completions(const std::string& uri, const Position& pos);
    std::optional<Hover> get_hover(const std::string& uri, const Position& pos);
    std::optional<Location> get_definition(const std::string& uri, const Position& pos);
    std::vector<DocumentSymbol> get_document_symbols(const std::string& uri);
    std::optional<SignatureHelp> get_signature_help(const std::string& uri, const Position& pos);
    std::vector<Location> get_references(const std::string& uri, const Position& pos);
    std::vector<Diagnostic> get_diagnostics(const std::string& uri);

    const std::string& get_source(const std::string& uri) const;

private:
    struct DocumentState {
        std::string source;
        std::vector<Token> tokens;
        std::unique_ptr<NodoAST> ast;
        std::vector<Diagnostic> diagnostics;
        std::vector<Symbol> symbols;
    };

    BuiltinRegistry builtins_;
    std::map<std::string, DocumentState> documents_;

    void walk_ast(const NodoAST* node, const std::string& uri,
                  std::vector<Symbol>& symbols, std::vector<Diagnostic>& diagnostics,
                  int depth = 0);

    void add_symbol(std::vector<Symbol>& symbols, const std::string& name,
                    SymbolKind kind, const std::string& uri, int line, int col,
                    const std::string& type_hint = "",
                    const std::vector<std::string>& params = {},
                    const std::string& doc = "");

    void add_diagnostic(std::vector<Diagnostic>& diagnostics,
                        int line, int col, int end_col,
                        int severity, const std::string& message);

    bool position_in_token(const Position& pos, const Token& tok) const;
    const Token* token_at_position(const std::string& uri, const Position& pos) const;
    const Symbol* find_symbol_at(const std::string& uri, const Position& pos) const;

    std::string token_type_name(TipoNodoAST tipo) const;
};

} // namespace zeta_lsp
