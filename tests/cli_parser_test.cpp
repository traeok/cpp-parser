#include <cassert>
#include <iostream>
#include <sstream>
#include "lexer/lexer.h"
#include "parser/cli_parser.h"
#include "lexer/source.h"

// Helper function to tokenize a string
std::vector<lexer::Token> tokenize(const std::string &input)
{
    lexer::Source source = lexer::Source::fromString(input, "<stdin>");
    return lexer::Lexer::tokenize(source);
}

// Helper function to print a command structure
void printCommand(const parser::CLICommand &cmd, int indent = 0)
{
    std::string spaces(indent * 2, ' ');

    std::cout << spaces << "Command: " << cmd.name << "\n";

    if (!cmd.flags.empty())
    {
        std::cout << spaces << "Flags:\n";
        for (const auto &flag : cmd.flags)
        {
            std::cout << spaces << "  " << (flag->isLong ? "--" : "-") << flag->name;
            if (!flag->value.empty())
            {
                std::cout << "=" << flag->value;
            }
            std::cout << "\n";
        }
    }

    if (!cmd.args.empty())
    {
        std::cout << spaces << "Arguments:\n";
        for (const auto &arg : cmd.args)
        {
            std::cout << spaces << "  " << arg->value << "\n";
        }
    }

    if (!cmd.subcommands.empty())
    {
        std::cout << spaces << "Subcommands:\n";
        for (const auto &subcmd : cmd.subcommands)
        {
            printCommand(*subcmd, indent + 1);
        }
    }
}

// Test cases
void testSimpleCommand()
{
    std::cout << "\nTesting simple command...\n";
    auto tokens = tokenize("git status");
    auto cmd = parser::CLIParser::parse(tokens);
    assert(cmd->name == "git");
    assert(cmd->args.size() == 1);
    assert(cmd->args[0]->value == "status");
    printCommand(*cmd);
}

void testCommandWithFlags()
{
    std::cout << "\nTesting command with flags...\n";
    auto tokens = tokenize("git commit -m \"Initial commit\" --verbose");
    auto cmd = parser::CLIParser::parse(tokens);
    assert(cmd->name == "git");
    assert(cmd->flags.size() == 2);
    assert(cmd->flags[0]->isLong == false);
    assert(cmd->flags[0]->name == "m");
    assert(cmd->flags[0]->value == "Initial commit");
    assert(cmd->flags[1]->isLong == true);
    assert(cmd->flags[1]->name == "verbose");
    printCommand(*cmd);
}

void testCommandWithEqualsFlags()
{
    std::cout << "\nTesting command with equals in flags...\n";
    auto tokens = tokenize("program --config=config.json --count=5");
    auto cmd = parser::CLIParser::parse(tokens);
    assert(cmd->name == "program");
    assert(cmd->flags.size() == 2);
    assert(cmd->flags[0]->name == "config");
    assert(cmd->flags[0]->value == "config.json");
    assert(cmd->flags[1]->name == "count");
    assert(cmd->flags[1]->value == "5");
    printCommand(*cmd);
}

void testComplexCommand()
{
    std::cout << "\nTesting complex command...\n";
    auto tokens = tokenize("docker run -p 8080:80 --name web-server --env-file .env ubuntu bash");
    auto cmd = parser::CLIParser::parse(tokens);
    assert(cmd->name == "docker");
    assert(cmd->args.size() == 2);
    assert(cmd->args[0]->value == "ubuntu");
    assert(cmd->args[1]->value == "bash");
    assert(cmd->flags.size() == 3);
    printCommand(*cmd);
}

void testErrorCases()
{
    std::cout << "\nTesting error cases...\n";

    // Test missing command name
    try
    {
        auto tokens = tokenize("--flag");
        auto cmd = parser::CLIParser::parse(tokens);
        assert(false && "Should have thrown an exception");
    }
    catch (const std::runtime_error &e)
    {
        std::cout << "Caught expected error: " << e.what() << "\n";
    }

    // Test invalid flag format
    try
    {
        auto tokens = tokenize("cmd --");
        auto cmd = parser::CLIParser::parse(tokens);
        assert(false && "Should have thrown an exception");
    }
    catch (const std::runtime_error &e)
    {
        std::cout << "Caught expected error: " << e.what() << "\n";
    }
}

int main()
{
    try
    {
        std::cout << "Running CLI parser tests...\n";

        testSimpleCommand();
        testCommandWithFlags();
        testCommandWithEqualsFlags();
        testComplexCommand();
        testErrorCases();

        std::cout << "\nAll tests passed!\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Test failed: " << e.what() << "\n";
        return 1;
    }
}