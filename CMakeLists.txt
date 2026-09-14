cmake_minimum_required(VERSION 3.15)
project(Calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# --- Calculator executable ---
add_executable(calculator
    src/main.cpp
    src/tokenizer.cpp
    src/parser.cpp
    src/graph_module.cpp
)

# Link SFML to calculator
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
target_link_libraries(calculator PRIVATE sfml-graphics sfml-window sfml-system)

# --- Tests executable ---
add_executable(tests
    src/tests.cpp
    src/tokenizer.cpp
    src/parser.cpp
)

# Link SFML to tests
target_link_libraries(tests PRIVATE sfml-graphics sfml-window sfml-system)
