#include <iostream>
#include <string>
#include <vector>
#include "lexer.hpp"

std::vector<lexer::Token> showTokens(const std::string &code)
{
    try
    {
        lexer::Source source = lexer::Source::fromString(code, "<stdin>");
        std::vector<lexer::Token> tokens = lexer::Lexer::tokenize(source);

        std::cout << "Tokens:" << std::endl;
        for (size_t i = 0; i < tokens.size(); ++i)
        {
            std::cout << "   ";
            tokens[i].print(std::cout);
            std::cout << std::endl;
        }
        return tokens;
    }
    catch (const lexer::LexError &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    std::cout << std::endl;
    return std::vector<lexer::Token>();
}

int main(int argc, char *argv[])
{
    // Register command handlers
    // In a real implementation, the registration would be more robust
    // and would actually store the handlers

    // Run in interactive mode if no arguments provided
    if (argc < 2)
    {
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "CLI Lexer Demo" << std::endl;
        std::cout << "Type a command or 'exit' to quit." << std::endl;
        std::cout << "Try 'help' for available commands." << std::endl;
        std::cout << "----------------------------------------" << std::endl;

        std::string line;
        while (true)
        {
            std::cout << "> ";
            std::getline(std::cin, line);

            if (line.empty() || line == "exit" || line == "quit")
            {
                break;
            }

            auto tokens = showTokens(line);
        }

        std::cout << "Goodbye!" << std::endl;
        return 0;
    }
}