# pparser

A lexical analyzer (lexer) and parser facility implemented in C++. This project started as a port of a Rust lexer to C++, following pre-C++11 standards with limited use of C++11 features (only vector, unordered_map, and map) to enable compatibility with older compilers.

## Lexer Features

- Tokenizes source code into a stream of tokens
- Supports various token types:
  - Keywords (if, else, for, etc.)
  - Operators (+, -, *, /, etc.)
  - Symbols (parentheses, braces, etc.)
  - Identifiers
  - Integer literals (decimal, hexadecimal, binary)
  - String literals
- Handles comments and whitespace
- Error reporting with location information

## Parser Features

- Parses tokens into an abstract syntax tree (AST)
- Supports various node types:
  - Assignment
  - Binary operation
  - Function call
  - Variable declaration
  - Integer literal
  - String literal
- Handles precedence and associativity
- Error reporting with location information

## Building the Lexer Demo

### Prerequisites

- CMake (version 2.8 or higher)
- C++ compiler with C++03 support

### Build Steps

```bash
# Generate build files
cmake .

# Build the project
cmake --build .

# Run the lexer demo
./Debug/lexer_demo
```

## Usage

When you run the lexer demo, you can type code snippets and see how they are tokenized:

```
> let x = 42;
Tokens:
   let
   x
   =
   42
   ;
```

Enter an empty line to exit the program.

## License

This project is open source and available under the MIT License. 