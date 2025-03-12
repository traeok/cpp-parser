#include "parser/cli_parser.h"
#include <stdexcept>

namespace parser
{

    CLIParser::CLIParser(const std::vector<lexer::Token> &tokens)
        : tokens(tokens), index(0) {}

    std::shared_ptr<CLICommand> CLIParser::parse(const std::vector<lexer::Token> &tokens)
    {
        CLIParser parser(tokens);
        return parser.parseCommand();
    }

    lexer::TokenKind CLIParser::cur() const
    {
        if (index < tokens.size())
        {
            return tokens[index].getKind();
        }
        return lexer::TOK_EOF;
    }

    void CLIParser::next()
    {
        if (index < tokens.size())
        {
            index++;
        }
    }

    const lexer::Token &CLIParser::currentToken() const
    {
        if (index >= tokens.size())
        {
            throw std::runtime_error("Unexpected end of input");
        }
        return tokens[index];
    }

    std::shared_ptr<CLICommand> CLIParser::parseCommand()
    {
        // First token should be an identifier (the command name)
        if (cur() != lexer::TOK_ID)
        {
            throw std::runtime_error("Expected command name");
        }

        auto command = std::make_shared<CLICommand>(currentToken().getId());
        next();

        // Parse flags and arguments until we hit EOF or a subcommand
        while (cur() != lexer::TOK_EOF)
        {
            if (cur() == lexer::TOK_MINUS || cur() == lexer::TOK_DOUBLE_MINUS)
            {
                // This is a flag
                command->flags.push_back(parseFlag());
            }
            else if (cur() == lexer::TOK_ID)
            {
                // This could be either a subcommand or an argument
                // For now, treat it as an argument
                command->args.push_back(parseArgument());
            }
            else
            {
                // Treat anything else as an argument
                command->args.push_back(parseArgument());
            }
        }

        return command;
    }

    std::shared_ptr<CLIFlag> CLIParser::parseFlag()
    {
        bool isLong = false;
        std::string name;

        // Check flag type (short or long)
        if (cur() == lexer::TOK_DOUBLE_MINUS)
        {
            isLong = true;
            next();
            if (cur() != lexer::TOK_ID)
            {
                throw std::runtime_error("Expected flag name after --");
            }
            name = currentToken().getId();
            next();
        }
        else if (cur() == lexer::TOK_MINUS)
        {
            next();
            if (cur() != lexer::TOK_ID)
            {
                throw std::runtime_error("Expected flag name after -");
            }
            name = currentToken().getId();
            next();
        }
        else
        {
            throw std::runtime_error("Expected flag");
        }

        // Check for flag value
        if (cur() == lexer::TOK_ASSIGN)
        {
            next();
            // Value can be an identifier, string literal, or number
            if (cur() == lexer::TOK_ID || cur() == lexer::TOK_STR_LIT || cur() == lexer::TOK_INT_LIT)
            {
                std::string value;
                if (cur() == lexer::TOK_STR_LIT)
                {
                    value = currentToken().getStrLit();
                }
                else if (cur() == lexer::TOK_INT_LIT)
                {
                    value = std::to_string(currentToken().getIntValue());
                }
                else
                {
                    value = currentToken().getId();
                }
                next();
                return std::make_shared<CLIFlag>(isLong, name, value);
            }
            else
            {
                throw std::runtime_error("Expected value after =");
            }
        }
        // Check for space-separated value
        else if (cur() == lexer::TOK_ID || cur() == lexer::TOK_STR_LIT || cur() == lexer::TOK_INT_LIT)
        {
            std::string value;
            if (cur() == lexer::TOK_STR_LIT)
            {
                value = currentToken().getStrLit();
            }
            else if (cur() == lexer::TOK_INT_LIT)
            {
                value = std::to_string(currentToken().getIntValue());
            }
            else
            {
                value = currentToken().getId();
            }
            next();
            return std::make_shared<CLIFlag>(isLong, name, value);
        }

        return std::make_shared<CLIFlag>(isLong, name);
    }

    std::shared_ptr<CLIArgument> CLIParser::parseArgument()
    {
        std::string value;

        switch (cur())
        {
        case lexer::TOK_ID:
            value = currentToken().getId();
            break;
        case lexer::TOK_STR_LIT:
            value = currentToken().getStrLit();
            break;
        case lexer::TOK_INT_LIT:
            value = std::to_string(currentToken().getIntValue());
            break;
        default:
            throw std::runtime_error("Expected argument");
        }

        next();
        return std::make_shared<CLIArgument>(value);
    }

} // namespace parser