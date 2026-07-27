#include "zeta/lexer.hpp"
#include "zeta/parser.hpp"
#include "zeta/interpreter.hpp"
#include "zeta/serializador.hpp"
#include "zeta/errores.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Zeta Language v0.1.0" << std::endl;
        std::cout << "Uso: zeta <archivo.zl> [opciones]" << std::endl;
        std::cout << std::endl;
        std::cout << "Opciones:" << std::endl;
        std::cout << "  --help, -h       Mostrar esta ayuda" << std::endl;
        std::cout << "  --debug, -d      Mostrar informacion de debug al finalizar" << std::endl;
        std::cout << "  --version, -v    Mostrar version" << std::endl;
        std::cout << std::endl;
        std::cout << "Ejemplos:" << std::endl;
        std::cout << "  zeta script.zl              Ejecutar un script" << std::endl;
        std::cout << "  zeta script.zl --debug      Ejecutar con debug" << std::endl;
        std::cout << std::endl;
        std::cout << "Documentacion: https://github.com/juan1417/zeta" << std::endl;
        return 0;
    }

    bool debug = false;
    std::string archivo_path;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--debug" || arg == "-d") {
            debug = true;
        } else if (arg == "--help" || arg == "-h") {
            // Reprint help when explicitly asked
            std::cout << "Zeta Language v0.1.0" << std::endl;
            std::cout << "Uso: zeta <archivo.zl> [opciones]" << std::endl;
            std::cout << std::endl;
            std::cout << "Opciones:" << std::endl;
            std::cout << "  --help, -h       Mostrar esta ayuda" << std::endl;
            std::cout << "  --debug, -d      Mostrar informacion de debug al finalizar" << std::endl;
            std::cout << "  --version, -v    Mostrar version" << std::endl;
            return 0;
        } else if (arg == "--version" || arg == "-v") {
            std::cout << "zeta v0.1.0" << std::endl;
            return 0;
        } else if (arg[0] != '-') {
            archivo_path = arg;
        } else {
            std::cerr << "Opcion desconocida: " << arg << std::endl;
            std::cerr << "Usa --help para ver las opciones disponibles." << std::endl;
            return 1;
        }
    }

    if (archivo_path.empty()) {
        std::cerr << "Error: no se especifico archivo" << std::endl;
        return 1;
    }

    std::ifstream archivo(archivo_path);
    if (!archivo.is_open()) {
        std::cerr << "Error: no se pudo abrir el archivo " << archivo_path << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << archivo.rdbuf();
    std::string codigo = buffer.str();

    try {
        zeta::Lexer lexer(std::move(codigo));
        auto tokens = lexer.tokenizar();

        zeta::Parser parser(std::move(tokens));
        auto ast = parser.parsear();

        zeta::Interpreter interpreter;
        interpreter.set_script_path(archivo_path);
        auto resultado = interpreter.ejecutar(std::move(ast));

        if (zeta::is_error(resultado)) {
            auto& err = zeta::obtener_error(resultado);
            std::cerr << "Error [" << err.tipo << "]: " << err.mensaje;
            if (err.linea > 0) std::cerr << " (linea " << err.linea << ")";
            std::cerr << std::endl;
            return 1;
        }

        if (debug) {
            std::cout << "\n=== DEBUG: Datos Internos ===" << std::endl;

            auto metricas = interpreter.obtener_metricas_json();
            if (!metricas.empty()) {
                std::cout << "\n--- Metricas KPI ---" << std::endl;
                for (const auto& m : metricas) {
                    std::cout << "  " << m.dump(2) << std::endl;
                }
            }

            auto dashboard = interpreter.obtener_dashboard_json();
            if (!dashboard.empty()) {
                std::cout << "\n--- Dashboard Config ---" << std::endl;
                std::cout << dashboard.dump(2) << std::endl;
            }

            auto datos = interpreter.obtener_datos_json();
            if (!datos.empty()) {
                std::cout << "\n--- Variables Globales ---" << std::endl;
                for (auto& [nombre, valor] : datos.items()) {
                    if (valor.is_array() && valor.size() > 10) {
                        std::cout << "  " << nombre << ": [array(" << valor.size() << ")]" << std::endl;
                    } else {
                        std::cout << "  " << nombre << ": " << valor.dump(2) << std::endl;
                    }
                }
            }

            std::cout << "\n=== Fin Debug ===" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error fatal: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
