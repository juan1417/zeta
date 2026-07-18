#include "zeta/xlsx_reader.hpp"
#include "deps/pugixml/src/pugixml.hpp"
#include "deps/miniz/miniz.h"
#include <string>
#include <vector>
#include <map>
#include <charconv>

namespace zeta {

static std::string zip_entry_to_string(mz_zip_archive* zip, const char* name) {
    mz_uint idx = mz_zip_reader_locate_file(zip, name, nullptr, 0);
    if (idx == (mz_uint)-1) return "";
    size_t size = 0;
    void* data = mz_zip_reader_extract_to_heap(zip, idx, &size, 0);
    if (!data) return "";
    std::string result(static_cast<char*>(data), size);
    mz_free(data);
    return result;
}

static int col_letter_to_index(const std::string& col) {
    int idx = 0;
    for (char c : col) {
        if (c >= 'A' && c <= 'Z') idx = idx * 26 + (c - 'A' + 1);
        else if (c >= 'a' && c <= 'z') idx = idx * 26 + (c - 'a' + 1);
    }
    return idx - 1;
}

DataFrame load_xlsx_file(const std::string& ruta, int sheet_index) {
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_reader_init_file(&zip, ruta.c_str(), 0)) {
        return DataFrame();
    }

    std::string shared_xml = zip_entry_to_string(&zip, "xl/sharedStrings.xml");
    std::vector<std::string> shared_strings;
    if (!shared_xml.empty()) {
        pugi::xml_document ss_doc;
        if (ss_doc.load_string(shared_xml.c_str())) {
            for (auto si : ss_doc.child("sst").children("si")) {
                std::string val;
                for (auto t : si.children("t")) val += t.child_value();
                shared_strings.push_back(val);
            }
        }
    }

    std::string sheet_file = "xl/worksheets/sheet" + std::to_string(sheet_index + 1) + ".xml";
    std::string sheet_xml = zip_entry_to_string(&zip, sheet_file.c_str());
    mz_zip_reader_end(&zip);

    if (sheet_xml.empty()) return DataFrame();

    pugi::xml_document doc;
    if (!doc.load_string(sheet_xml.c_str())) return DataFrame();

    auto sheet = doc.child("worksheet");
    if (!sheet) return DataFrame();

    // Find dimensions
    int max_col = 0;
    int max_row = 0;
    auto dims = sheet.child("dimension");
    if (dims) {
        std::string ref = dims.attribute("ref").as_string();
        size_t colon = ref.find(':');
        if (colon != std::string::npos) {
            std::string end_ref = ref.substr(colon + 1);
            std::string end_col_str, end_row_str;
            for (char c : end_ref) {
                if (std::isalpha(c)) end_col_str += c;
                else end_row_str += c;
            }
            max_col = col_letter_to_index(end_col_str) + 1;
            max_row = std::stoi(end_row_str);
        }
    }

    if (max_col == 0 || max_row == 0) {
        auto sheet_data = sheet.child("sheetData");
        if (!sheet_data) return DataFrame();
        for (auto row : sheet_data.children("row")) {
            int r = row.attribute("r").as_int();
            if (r > max_row) max_row = r;
            for (auto cell : row.children("c")) {
                std::string ref = cell.attribute("r").as_string();
                std::string col_str;
                for (char c : ref) {
                    if (std::isalpha(c)) col_str += c;
                    else break;
                }
                int ci = col_letter_to_index(col_str) + 1;
                if (ci > max_col) max_col = ci;
            }
        }
    }

    if (max_col == 0 || max_row == 0) return DataFrame();

    // Read cells into matrix
    std::vector<std::vector<std::string>> raw(max_col, std::vector<std::string>(max_row, ""));
    auto sheet_data = sheet.child("sheetData");
    if (!sheet_data) return DataFrame();

    for (auto row : sheet_data.children("row")) {
        for (auto cell : row.children("c")) {
            std::string ref = cell.attribute("r").as_string();
            std::string col_str, row_str;
            for (char c : ref) {
                if (std::isalpha(c)) col_str += c;
                else row_str += c;
            }
            int ci = col_letter_to_index(col_str);
            int ri = std::stoi(row_str) - 1;
            if (ci < 0 || ci >= max_col || ri < 0 || ri >= max_row) continue;

            std::string cell_type = cell.attribute("t").as_string("n");
            std::string val = cell.child("v").child_value();
            if (cell_type == "s") {
                int idx = std::stoi(val);
                if (idx >= 0 && idx < (int)shared_strings.size())
                    raw[ci][ri] = shared_strings[idx];
                else
                    raw[ci][ri] = "";
            } else if (cell_type == "b") {
                raw[ci][ri] = val == "1" ? "true" : "false";
            } else {
                raw[ci][ri] = val;
            }
        }
    }

    // Build headers from first row (row 1)
    std::vector<std::string> headers;
    for (int ci = 0; ci < max_col; ++ci) {
        headers.push_back(raw[ci][0]);
    }

    // Infer types and build columns (rows 2..max_row are data)
    std::map<std::string, Columna> columnas;
    for (int ci = 0; ci < max_col; ++ci) {
        int tipo_detectado = 0;
        for (int ri = 1; ri < max_row; ++ri) {
            const auto& v = raw[ci][ri];
            if (v.empty() || v == "null") continue;
            if (v == "true" || v == "false" || v == "TRUE" || v == "FALSE") {
                tipo_detectado = 2; break;
            }
            double dummy;
            auto [ptr, ec] = std::from_chars(v.data(), v.data() + v.size(), dummy);
            if (ec == std::errc() && ptr == v.data() + v.size()) {
                tipo_detectado = 1; break;
            }
            tipo_detectado = 3; break;
        }

        int data_rows = max_row - 1;
        if (tipo_detectado == 1) {
            Columna c("num");
            c.nums.reserve(data_rows);
            c.null_bitmap.reserve(data_rows);
            for (int ri = 1; ri < max_row; ++ri) {
                const auto& v = raw[ci][ri];
                if (v.empty() || v == "null") {
                    c.nums.push_back(crear_null());
                    c.null_bitmap.push_back(true);
                } else {
                    double val;
                    auto [ptr, ec] = std::from_chars(v.data(), v.data() + v.size(), val);
                    c.nums.push_back(ec == std::errc() ? val : crear_null());
                    c.null_bitmap.push_back(ec != std::errc());
                }
            }
            columnas[headers[ci]] = std::move(c);
        } else if (tipo_detectado == 2) {
            Columna c("bool");
            c.bools.reserve(data_rows);
            c.null_bitmap.reserve(data_rows);
            for (int ri = 1; ri < max_row; ++ri) {
                const auto& v = raw[ci][ri];
                if (v.empty() || v == "null") {
                    c.bools.push_back(false);
                    c.null_bitmap.push_back(true);
                } else {
                    c.bools.push_back(v == "true" || v == "TRUE");
                    c.null_bitmap.push_back(false);
                }
            }
            columnas[headers[ci]] = std::move(c);
        } else {
            Columna c("str");
            c.strs.reserve(data_rows);
            c.null_bitmap.reserve(data_rows);
            for (int ri = 1; ri < max_row; ++ri) {
                const auto& v = raw[ci][ri];
                if (v.empty() || v == "null") {
                    c.strs.push_back("");
                    c.null_bitmap.push_back(true);
                } else {
                    c.strs.push_back(v);
                    c.null_bitmap.push_back(false);
                }
            }
            columnas[headers[ci]] = std::move(c);
        }
    }

    DataFrame df;
    df.nombres_columnas = std::move(headers);
    df.columnas = std::move(columnas);
    return df;
}

std::string save_xlsx_file(const std::string& ruta, const DataFrame& df) {
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_writer_init_file(&zip, ruta.c_str(), 0)) {
        return "Error: no se pudo crear " + ruta;
    }

    // Build shared strings and strings index
    std::map<std::string, int> str_index;
    std::vector<std::string> shared;

    auto add_shared = [&](const std::string& s) -> int {
        auto it = str_index.find(s);
        if (it != str_index.end()) return it->second;
        int idx = shared.size();
        shared.push_back(s);
        str_index[s] = idx;
        return idx;
    };

    // Collect initial string values from str columns
    for (const auto& [name, col] : df.columnas) {
        if (col.tipo == "str") {
            for (const auto& s : col.strs) add_shared(s);
        }
    }

    std::string sheet_xml;
    {
        size_t nrows = 0;
        for (const auto& [name, col] : df.columnas) {
            size_t s = col.size();
            if (s > nrows) nrows = s;
        }

        sheet_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";
        sheet_xml += "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">";
        sheet_xml += "<sheetData>";

        auto col_letter = [](int idx) -> std::string {
            std::string s;
            idx++;
            while (idx > 0) {
                idx--;
                s = (char)('A' + idx % 26) + s;
                idx /= 26;
            }
            return s;
        };

        // Header row
        sheet_xml += "<row r=\"1\">";
        int ci = 0;
        std::vector<std::string> col_names;
        for (const auto& [name, col] : df.columnas) col_names.push_back(name);
        for (const auto& name : col_names) {
            sheet_xml += "<c r=\"" + col_letter(ci) + "1\" t=\"s\"><v>" + std::to_string(add_shared(name)) + "</v></c>";
            ci++;
        }
        sheet_xml += "</row>";

        // Data rows
        for (size_t ri = 0; ri < nrows; ++ri) {
            sheet_xml += "<row r=\"" + std::to_string(ri + 2) + "\">";
            ci = 0;
            for (const auto& name : col_names) {
                const auto& col = df.columnas.at(name);
                std::string ref = col_letter(ci) + std::to_string(ri + 2);
                if (col.null_bitmap[ri]) {
                    sheet_xml += "<c r=\"" + ref + "\"><v></v></c>";
                } else if (col.tipo == "num") {
                    char buf[64];
                    auto [ptr, ec] = std::to_chars(buf, buf + 64, col.nums[ri], std::chars_format::general, 15);
                    std::string val(buf, ptr - buf);
                    sheet_xml += "<c r=\"" + ref + "\"><v>" + val + "</v></c>";
                } else if (col.tipo == "bool") {
                    sheet_xml += "<c r=\"" + ref + "\" t=\"b\"><v>" + std::to_string(col.bools[ri] ? 1 : 0) + "</v></c>";
                } else {
                    int idx = add_shared(col.strs[ri]);
                    sheet_xml += "<c r=\"" + ref + "\" t=\"s\"><v>" + std::to_string(idx) + "</v></c>";
                }
                ci++;
            }
            sheet_xml += "</row>";
        }
        sheet_xml += "</sheetData></worksheet>";
    }

    // Now build sharedStrings.xml with all strings collected
    {
        std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";
        xml += "<sst xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" count=\"0\" uniqueCount=\"" + std::to_string(shared.size()) + "\">";
        for (const auto& s : shared) {
            xml += "<si><t>" + s + "</t></si>";
        }
        xml += "</sst>";
        mz_zip_writer_add_mem(&zip, "xl/sharedStrings.xml", xml.c_str(), xml.size(), MZ_DEFAULT_COMPRESSION);
    }

    mz_zip_writer_add_mem(&zip, "xl/worksheets/sheet1.xml", sheet_xml.c_str(), sheet_xml.size(), MZ_DEFAULT_COMPRESSION);

    // Minimal content types
    {
        std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";
        xml += "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">";
        xml += "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>";
        xml += "<Default Extension=\"xml\" ContentType=\"application/xml\"/>";
        xml += "<Override PartName=\"/xl/workbooks.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>";
        xml += "<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>";
        xml += "<Override PartName=\"/xl/sharedStrings.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml\"/>";
        xml += "</Types>";
        mz_zip_writer_add_mem(&zip, "[Content_Types].xml", xml.c_str(), xml.size(), MZ_DEFAULT_COMPRESSION);
    }

    // Minimal workbook
    {
        std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";
        xml += "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">";
        xml += "<sheets><sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"rId1\"/></sheets>";
        xml += "</workbook>";
        mz_zip_writer_add_mem(&zip, "xl/workbook.xml", xml.c_str(), xml.size(), MZ_DEFAULT_COMPRESSION);
    }

    // Minimal rels
    {
        std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";
        xml += "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">";
        xml += "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet1.xml\"/>";
        xml += "<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings\" Target=\"sharedStrings.xml\"/>";
        xml += "</Relationships>";
        mz_zip_writer_add_mem(&zip, "xl/_rels/workbook.xml.rels", xml.c_str(), xml.size(), MZ_DEFAULT_COMPRESSION);
    }

    {
        std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";
        xml += "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">";
        xml += "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>";
        xml += "</Relationships>";
        mz_zip_writer_add_mem(&zip, "_rels/.rels", xml.c_str(), xml.size(), MZ_DEFAULT_COMPRESSION);
    }

    mz_zip_writer_finalize_archive(&zip);
    mz_zip_writer_end(&zip);
    return "Guardado: " + ruta;
}

} // namespace zeta
