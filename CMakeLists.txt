cmake_minimum_required(VERSION 3.15)
project(zeta LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

find_package(Threads REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/core/valor_zeta.cpp
    src/core/tabla_simbolos.cpp
    src/core/estadisticas.cpp
    src/core/errores.cpp
    src/core/serializador.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/interpreter/interpreter.cpp
)

add_executable(zeta ${SOURCES})

target_include_directories(zeta PRIVATE include)
target_link_libraries(zeta PRIVATE nlohmann_json::nlohmann_json)

# Server executable
set(SERVER_SOURCES
    src/server_main.cpp
    src/core/valor_zeta.cpp
    src/core/tabla_simbolos.cpp
    src/core/estadisticas.cpp
    src/core/errores.cpp
    src/core/serializador.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/interpreter/interpreter.cpp
)

add_executable(zeta_server ${SERVER_SOURCES})

target_include_directories(zeta_server PRIVATE include deps deps/asio)
target_compile_definitions(zeta_server PRIVATE ASIO_STANDALONE)
target_link_libraries(zeta_server PRIVATE nlohmann_json::nlohmann_json Threads::Threads)

# Tests
enable_testing()
add_executable(test_zeta tests/test_core.cpp src/core/valor_zeta.cpp src/core/estadisticas.cpp src/core/errores.cpp src/core/serializador.cpp)
target_include_directories(test_zeta PRIVATE include)
target_link_libraries(test_zeta PRIVATE nlohmann_json::nlohmann_json)
add_test(NAME test_core COMMAND test_zeta)
