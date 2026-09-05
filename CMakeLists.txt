cmake_minimum_required(VERSION 3.15)
project(Calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(calculator
    src/main.cpp
    src/tokenizer.cpp
    src/parser.cpp
)

add_executable(tests
    src/tests.cpp
    src/tokenizer.cpp
    src/parser.cpp
)
