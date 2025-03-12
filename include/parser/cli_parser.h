#ifndef PARSER_CLI_PARSER_H
#define PARSER_CLI_PARSER_H

#include <vector>
#include <memory>
#include <string>
#include "lexer/token.h"

namespace parser
{

    // Forward declarations
    class CLICommand;
    class CLIFlag;
    class CLIArgument;

    // Base class for CLI AST nodes
    class CLINode
    {
    public:
        virtual ~CLINode() = default;
    };

    // Represents a CLI command with its flags and arguments
    class CLICommand : public CLINode
    {
    public:
        std::string name;
        std::vector<std::shared_ptr<CLIFlag>> flags;
        std::vector<std::shared_ptr<CLIArgument>> args;
        std::vector<std::shared_ptr<CLICommand>> subcommands;

        CLICommand(const std::string &name) : name(name) {}
    };

    // Represents a CLI flag (both short and long forms)
    class CLIFlag : public CLINode
    {
    public:
        bool isLong;
        std::string name;
        std::string value; // Optional value for flags that take values

        CLIFlag(bool isLong, const std::string &name)
            : isLong(isLong), name(name) {}

        CLIFlag(bool isLong, const std::string &name, const std::string &value)
            : isLong(isLong), name(name), value(value) {}
    };

    // Represents a positional argument
    class CLIArgument : public CLINode
    {
    public:
        std::string value;

        CLIArgument(const std::string &value) : value(value) {}
    };

    // The CLI parser class
    class CLIParser
    {
    public:
        static std::shared_ptr<CLICommand> parse(const std::vector<lexer::Token> &tokens);

    private:
        CLIParser(const std::vector<lexer::Token> &tokens);

        std::shared_ptr<CLICommand> parseCommand();
        std::shared_ptr<CLIFlag> parseFlag();
        std::shared_ptr<CLIArgument> parseArgument();

        const std::vector<lexer::Token> &tokens;
        size_t index;

        lexer::TokenKind cur() const;
        void next();
        const lexer::Token &currentToken() const;
    };

} // namespace parser

#endif // PARSER_CLI_PARSER_H