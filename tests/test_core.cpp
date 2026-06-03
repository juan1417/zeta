#include "zeta/valor_zeta.hpp"
#include "zeta/estadisticas.hpp"
#include "zeta/errores.hpp"
#include "zeta/serializador.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace zeta;

void test_null() {
    std::cout << "[TEST] Null handling... ";
    double n = crear_null();
    assert(es_null(n));
    assert(!es_null(42.0));
    assert(!es_null(0.0));
    assert(es_null(0.0 / 0.0));
    std::cout << "PASSED" << std::endl;
}

void test_mean() {
    std::cout << "[TEST] Mean calculation... ";
    std::vector<double> vec = {10.0, 20.0, 30.0, 40.0, 50.0};
    assert(std::abs(fn_mean(vec) - 30.0) < 0.001);

    std::vector<double> con_null = {10.0, crear_null(), 30.0};
    assert(std::abs(fn_mean(con_null) - 20.0) < 0.001);

    std::vector<double> vacio;
    assert(es_null(fn_mean(vacio)));
    std::cout << "PASSED" << std::endl;
}

void test_count() {
    std::cout << "[TEST] Count non-null... ";
    std::vector<double> vec = {1.0, crear_null(), 3.0, crear_null(), 5.0};
    assert(fn_count(vec) == 3);
    std::cout << "PASSED" << std::endl;
}

void test_sum() {
    std::cout << "[TEST] Sum calculation... ";
    std::vector<double> vec = {1.0, 2.0, 3.0};
    assert(std::abs(fn_sum(vec) - 6.0) < 0.001);

    std::vector<double> con_null = {1.0, crear_null(), 3.0};
    assert(std::abs(fn_sum(con_null) - 4.0) < 0.001);
    std::cout << "PASSED" << std::endl;
}

void test_min_max() {
    std::cout << "[TEST] Min/Max calculation... ";
    std::vector<double> vec = {5.0, 2.0, 8.0, 1.0, 9.0};
    assert(std::abs(fn_min(vec) - 1.0) < 0.001);
    assert(std::abs(fn_max(vec) - 9.0) < 0.001);

    std::vector<double> con_null = {5.0, crear_null(), 1.0};
    assert(std::abs(fn_min(con_null) - 1.0) < 0.001);
    assert(std::abs(fn_max(con_null) - 5.0) < 0.001);
    std::cout << "PASSED" << std::endl;
}

void test_stddev() {
    std::cout << "[TEST] Standard deviation... ";
    std::vector<double> vec = {2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};
    double sd = fn_stddev(vec);
    assert(std::abs(sd - 2.138) < 0.01);
    std::cout << "PASSED" << std::endl;
}

void test_is_null_vector() {
    std::cout << "[TEST] is_null vector... ";
    std::vector<double> vec = {1.0, crear_null(), 3.0, crear_null()};
    auto resultado = fn_is_null(vec);
    assert(resultado[0] == false);
    assert(resultado[1] == true);
    assert(resultado[2] == false);
    assert(resultado[3] == true);
    std::cout << "PASSED" << std::endl;
}

void test_rellenar_nulls() {
    std::cout << "[TEST] Fill nulls with mean... ";
    std::vector<double> vec = {10.0, crear_null(), 30.0};
    auto resultado = rellenar_nulls_con_media(vec);
    assert(!es_null(resultado[0]));
    assert(!es_null(resultado[1]));
    assert(resultado[1] == 20.0);
    std::cout << "PASSED" << std::endl;
}

void test_errors() {
    std::cout << "[TEST] Error system... ";
    auto err = crear_error("io", "Archivo no encontrado", 42);
    assert(is_error(err));
    auto& e = obtener_error(err);
    assert(e.tipo == "io");
    assert(e.mensaje == "Archivo no encontrado");
    assert(e.linea == 42);

    ValorZeta ok = mk_num(42.0);
    assert(!is_error(ok));
    std::cout << "PASSED" << std::endl;
}

void test_dataframe() {
    std::cout << "[TEST] DataFrame creation... ";
    DataFrame df;
    df.columnas["id"] = {1.0, 2.0, 3.0};
    df.columnas["edad"] = {25.0, 31.0, crear_null()};
    df.columnas["score"] = {8.5, 9.2, 7.0};

    assert(df.validar_simetria());
    assert(df.filas() == 3);
    assert(df.columnas_count() == 3);
    std::cout << "PASSED" << std::endl;
}

void test_valor_factory() {
    std::cout << "[TEST] Valor factory functions... ";
    auto b = mk_bool(true);
    assert(b->tipo == ValorImpl::BOOL);
    assert(get_bool(b) == true);

    auto n = mk_num(42.5);
    assert(n->tipo == ValorImpl::NUM);
    assert(get_num(n) == 42.5);

    auto s = mk_str("hola");
    assert(s->tipo == ValorImpl::STR);
    assert(get_str(s) == "hola");

    auto v = mk_vec(std::vector<double>{1.0, 2.0});
    assert(v->tipo == ValorImpl::VEC);
    assert(get_vec(v).size() == 2);

    auto nl = mk_null_val();
    assert(es_null_valor(nl));
    std::cout << "PASSED" << std::endl;
}

void test_serialization() {
    std::cout << "[TEST] JSON serialization... ";
    auto num = mk_num(42.5);
    auto j_num = valor_a_json(num);
    assert(j_num == 42.5);

    auto nulo = mk_null_val();
    auto j_null = valor_a_json(nulo);
    assert(j_null.is_null());

    auto str = mk_str("hola");
    auto j_str = valor_a_json(str);
    assert(j_str == "hola");

    auto b = mk_bool(true);
    auto j_bool = valor_a_json(b);
    assert(j_bool == true);

    auto vec = mk_vec(std::vector<double>{1.0, crear_null(), 3.0});
    auto j_vec = valor_a_json(vec);
    assert(j_vec.is_array());
    assert(j_vec.size() == 3);
    assert(j_vec[0] == 1.0);
    assert(j_vec[1].is_null());
    assert(j_vec[2] == 3.0);

    DataFrame df;
    df.columnas["a"] = {1.0, 2.0};
    auto j_df = dataframe_a_json(df);
    assert(j_df.is_object());
    assert(j_df["a"].is_array());
    std::cout << "PASSED" << std::endl;
}

void test_tipo_nombre() {
    std::cout << "[TEST] Type names... ";
    assert(tipo_nombre(mk_bool(true)) == "bool");
    assert(tipo_nombre(mk_num(42.0)) == "double");
    assert(tipo_nombre(mk_str("x")) == "string");
    assert(tipo_nombre(mk_vec({})) == "vector");
    assert(tipo_nombre(crear_error("t", "m", 0)) == "error");
    std::cout << "PASSED" << std::endl;
}

int main() {
    std::cout << "=== Zeta Core Tests ===" << std::endl;

    test_null();
    test_mean();
    test_count();
    test_sum();
    test_min_max();
    test_stddev();
    test_is_null_vector();
    test_rellenar_nulls();
    test_errors();
    test_dataframe();
    test_valor_factory();
    test_serialization();
    test_tipo_nombre();

    std::cout << "\n=== All tests PASSED ===" << std::endl;
    return 0;
}
