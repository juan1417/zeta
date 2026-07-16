#pragma once
#include <string>
#include <vector>
#include <optional>
#include "json.hpp"

namespace zeta_lsp {

using json = nlohmann::json;

struct Position {
    int line = 0;
    int character = 0;
    json to_json() const { return {{"line", line}, {"character", character}}; }
    static Position from_json(const json& j) {
        return {j.value("line", 0), j.value("character", 0)};
    }
};

struct Range {
    Position start;
    Position end;
    json to_json() const { return {{"start", start.to_json()}, {"end", end.to_json()}}; }
    static Range from_json(const json& j) {
        return {Position::from_json(j["start"]), Position::from_json(j["end"])};
    }
};

struct Location {
    std::string uri;
    Range range;
    json to_json() const { return {{"uri", uri}, {"range", range.to_json()}}; }
};

struct TextDocumentIdentifier {
    std::string uri;
    json to_json() const { return {{"uri", uri}}; }
    static TextDocumentIdentifier from_json(const json& j) {
        return {j.value("uri", "")};
    }
};

struct TextDocumentPosition {
    TextDocumentIdentifier textDocument;
    Position position;
    static TextDocumentPosition from_json(const json& j) {
        return {TextDocumentIdentifier::from_json(j["textDocument"]),
                Position::from_json(j["position"])};
    }
};

enum class CompletionItemKind {
    Text = 1, Method = 2, Function = 3, Constructor = 4, Field = 5,
    Variable = 6, Class = 7, Interface = 8, Module = 9, Property = 10,
    Unit = 11, Value = 12, Enum = 13, Keyword = 14, Snippet = 15,
    Color = 16, File = 17, Reference = 18, EnumMember = 22, Struct = 23,
};

enum class SymbolKind {
    File = 1, Module = 2, Namespace = 3, Package = 4, Class = 5,
    Method = 6, Property = 7, Field = 8, Constructor = 9, Enum = 10,
    Interface = 11, Function = 12, Variable = 13, Constant = 14,
    String = 15, Number = 16, Boolean = 17, Array = 18, Object = 19,
    Key = 20, Null = 21, EnumMember = 22, Struct = 23,
};

enum class DiagnosticSeverity {
    Error = 1, Warning = 2, Information = 3, Hint = 4,
};

struct MarkupContent {
    std::string kind = "markdown";
    std::string value;
    json to_json() const { return {{"kind", kind}, {"value", value}}; }
};

struct CompletionItem {
    std::string label;
    CompletionItemKind kind = CompletionItemKind::Function;
    std::string detail;
    std::optional<MarkupContent> documentation;
    std::string insertText;
    int insertTextFormat = 1;
    json to_json() const {
        json j = {{"label", label}, {"kind", (int)kind}};
        if (!detail.empty()) j["detail"] = detail;
        if (documentation) j["documentation"] = documentation->to_json();
        if (!insertText.empty()) j["insertText"] = insertText;
        j["insertTextFormat"] = insertTextFormat;
        return j;
    }
};

struct Hover {
    MarkupContent contents;
    std::optional<Range> range;
    json to_json() const {
        json j = {{"contents", contents.to_json()}};
        if (range) j["range"] = range->to_json();
        return j;
    }
};

struct DocumentSymbol {
    std::string name;
    std::string detail;
    SymbolKind kind;
    Range range;
    Range selectionRange;
    std::vector<DocumentSymbol> children;
    json to_json() const {
        json j = {
            {"name", name}, {"kind", (int)kind},
            {"range", range.to_json()}, {"selectionRange", selectionRange.to_json()}
        };
        if (!detail.empty()) j["detail"] = detail;
        if (!children.empty()) {
            json ch = json::array();
            for (auto& c : children) ch.push_back(c.to_json());
            j["children"] = ch;
        }
        return j;
    }
};

struct ParameterInformation {
    std::string label;
    std::optional<MarkupContent> documentation;
    json to_json() const {
        json j = {{"label", label}};
        if (documentation) j["documentation"] = documentation->to_json();
        return j;
    }
};

struct SignatureInformation {
    std::string label;
    std::optional<MarkupContent> documentation;
    std::vector<ParameterInformation> parameters;
    json to_json() const {
        json j = {{"label", label}};
        if (documentation) j["documentation"] = documentation->to_json();
        json params = json::array();
        for (auto& p : parameters) params.push_back(p.to_json());
        j["parameters"] = params;
        return j;
    }
};

struct SignatureHelp {
    std::vector<SignatureInformation> signatures;
    int activeSignature = 0;
    int activeParameter = 0;
    json to_json() const {
        json sigs = json::array();
        for (auto& s : signatures) sigs.push_back(s.to_json());
        return {{"signatures", sigs},
                {"activeSignature", activeSignature},
                {"activeParameter", activeParameter}};
    }
};

struct Diagnostic {
    Range range;
    int severity = 1;
    std::string message;
    std::string source = "zeta-lsp";
    json to_json() const {
        return {{"range", range.to_json()}, {"severity", severity},
                {"message", message}, {"source", source}};
    }
};

struct DiagnosticParams {
    std::string uri;
    std::vector<Diagnostic> diagnostics;
    json to_json() const {
        json diags = json::array();
        for (auto& d : diagnostics) diags.push_back(d.to_json());
        return {{"uri", uri}, {"diagnostics", diags}};
    }
};

} // namespace zeta_lsp
