cmake_minimum_required(VERSION 3.5)
project(lexer-cpp)


# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/lexer/token.cpp
    src/lexer/source.cpp
    src/lexer/error.cpp
    src/lexer/lexer.cpp
    src/main.cpp
    src/parser/cli_parser.cpp
)

# Create executable
add_executable(lexer ${SOURCES})

# Set output directory
set_target_properties(lexer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Add compiler flags
if(MSVC)
    # MSVC flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    # GCC/Clang flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif() 

# Add tests
add_executable(cli_parser_test
    tests/cli_parser_test.cpp
)