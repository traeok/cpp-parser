project(cli_parser_test)
cmake_minimum_required(VERSION 3.5)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/../include)

add_executable(cli_parser_test
    cli_parser_test.cpp
)

target_link_libraries(cli_parser_test
    PRIVATE
    ../Debug/lexer
)

target_include_directories(cli_parser_test
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
) 