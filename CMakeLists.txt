cmake_minimum_required(VERSION 3.5)
project(lexer-cpp)

# Include directories
include_directories(include)

add_executable(lexer_demo src/lexer_demo.cpp)

# Set output directory
set_target_properties(lexer_demo PROPERTIES
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