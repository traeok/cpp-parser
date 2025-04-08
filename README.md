# pparser

A lexical analyzer (lexer) and CLI parser facility implemented in C++. This project started as a port of a Rust lexer to C++, following pre-C++11 standards with limited use of C++11 features (only vector, unordered_map, and map) to enable compatibility with older compilers.

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

- Structures commands using a tree of nodes
- Supports various CLI command structures:
  - optional and required flags
  - keyword arguments
  - positional arguments
  - subcommands
  - extensive numeric parsing (floating-point numbers, binary, hexadecimal, etc.)
  - and more!
- Handles precedence and associativity
- Error reporting with location information

## Building the demos

### Prerequisites

- CMake (version 2.8 or higher)
- C++ compiler with C++03 support

### Build steps

```bash
# Generate build files
cmake .

# Build the project
cmake --build .

# Run the lexer demo
./Debug/lexer_demo

# Run the parser demo
./Debug/pparser_demo
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

The parser demo is a git-like CLI with support for a couple subcommands and flags for each subcommand. You can type commands like:

```
> add file1.txt -f
> commit -m "Initial commit"
```

The parser parses the input and invokes the command handler for the recognized command. The handlers simply print the provided arguments and inputs to show how the parsed data can be used.

## License

This project is open source and available under the MIT License. 