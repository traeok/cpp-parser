#include "lexer.hpp"
#include <iostream>
#include <string>
#include <vector>

std::vector<lexer::Token> show_tokens(const std::string &code) {
  try {
    lexer::Src source = lexer::Src::from_string(code, "<stdin>");
    std::vector<lexer::Token> tokens = lexer::Lexer::tokenize(source);

    std::cout << "tokens:" << std::endl;
    for (size_t i = 0; i < tokens.size(); ++i) {
      std::cout << "   ";
      tokens[i].print(std::cout);
      std::cout << std::endl;
    }
    return tokens;
  } catch (const lexer::LexError &e) {
    std::cerr << "error: " << e.what() << std::endl;
  }

  std::cout << std::endl;
  return std::vector<lexer::Token>();
}

int main(int argc, char *argv[]) {
  std::cout << "----------------------------------------" << std::endl;
  std::cout << "lexer demo" << std::endl;
  std::cout << "type a string of tokens or type 'exit' to quit." << std::endl;
  std::cout << "----------------------------------------\n" << std::endl;

  std::string line;
  while (true) {
    std::cout << "> ";
    std::getline(std::cin, line);

    if (line.empty() || line == "exit" || line == "quit") {
      break;
    }

    auto tokens = show_tokens(line);
  }

  std::cout << "goodbye!" << std::endl;
  return 0;
}