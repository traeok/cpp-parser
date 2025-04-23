#include <cassert>
#include <iostream>
#include <sstream>
#include "../include/lexer.hpp"
#include "../include/pparser.hpp"

// Helper function to parse a command line using pparser
pparser::ParseResult parse_command_line(const std::string &input, pparser::ArgumentParser &parser)
{
    return parser.parse(input);
}

// Test cases
void testQuotedParenthesesArgument()
{
    std::cout << "\nTesting quoted argument with parentheses...\n";
    pparser::ArgumentParser parser("ds", "Test CLI parser");

    // root command: ds
    pparser::Command &root = parser.get_root_command();
    // add positional argument: view
    root.add_positional_arg("cmd", "subcommand", pparser::ArgType_Single, true);
    // add positional argument: data
    root.add_positional_arg("data", "data argument", pparser::ArgType_Single, true);

    // Print tokens for debugging
    std::string input = "view \"MY.DATA(MEMBER)\"";
    lexer::Src src = lexer::Src::from_string(input, "<test>");
    std::vector<lexer::Token> tokens = lexer::Lexer::tokenize(src);
    std::cout << "Tokens:\n";
    for (size_t i = 0; i < tokens.size(); ++i) {
        std::cout << "  [" << i << "] ";
        tokens[i].print(std::cout);
        std::cout << "\n";
    }
    std::cout << std::flush;

    pparser::ParseResult result = parse_command_line(input, parser);

    assert(result.status == pparser::ParseResult::ParserStatus_Success);
    assert(result.find_pos_arg_string("cmd") == "view");
    assert(result.find_pos_arg_string("data") == "MY.DATA(MEMBER)");
    std::cout << "cmd: " << result.find_pos_arg_string("cmd") << "\n";
    std::cout << "data: " << result.find_pos_arg_string("data") << "\n";
}

int main()
{
    try
    {
        std::cout << "Running CLI parser tests...\n";
        testQuotedParenthesesArgument();
        std::cout << "\nAll tests passed!\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Test failed: " << e.what() << "\n";
        return 1;
    }
}
