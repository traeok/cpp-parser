#include "lexer/token.h"
#include <cstring>
#include <iostream>
#include <string>

namespace lexer
{

    Token::Token() : kind(TOK_EOF)
    {
    }

    Token::Token(TokenKind kind, const Span &span)
        : kind(kind), span(span)
    {
    }

    // Implement copy constructor
    Token::Token(const Token &other)
        : kind(other.kind), span(other.span)
    {
        // Deep copy string data if needed
        if (other.kind == TOK_ID || other.kind == TOK_FLAG_SHORT ||
            other.kind == TOK_FLAG_LONG || other.kind == TOK_COMMAND)
        {
            data.id = other.data.id ? strdup(other.data.id) : NULL;
        }
        else if (other.kind == TOK_STR_LIT || other.kind == TOK_OPTION_VALUE ||
                 other.kind == TOK_ARGUMENT)
        {
            data.strLit = other.data.strLit ? strdup(other.data.strLit) : NULL;
        }
        else if (other.kind == TOK_INT_LIT)
        {
            data.intLit = other.data.intLit;
        }
    }

    // Implement assignment operator
    Token &Token::operator=(const Token &other)
    {
        if (this != &other)
        {
            // First deallocate any existing dynamic memory
            if (kind == TOK_ID || kind == TOK_FLAG_SHORT ||
                kind == TOK_FLAG_LONG || kind == TOK_COMMAND)
            {
                free((void *)data.id);
                data.id = NULL;
            }
            else if (kind == TOK_STR_LIT || kind == TOK_OPTION_VALUE ||
                     kind == TOK_ARGUMENT)
            {
                free((void *)data.strLit);
                data.strLit = NULL;
            }

            // Copy the kind and span
            kind = other.kind;
            span = other.span;

            // Deep copy string data if needed
            if (other.kind == TOK_ID || other.kind == TOK_FLAG_SHORT ||
                other.kind == TOK_FLAG_LONG || other.kind == TOK_COMMAND)
            {
                data.id = other.data.id ? strdup(other.data.id) : NULL;
            }
            else if (other.kind == TOK_STR_LIT || other.kind == TOK_OPTION_VALUE ||
                     other.kind == TOK_ARGUMENT)
            {
                data.strLit = other.data.strLit ? strdup(other.data.strLit) : NULL;
            }
            else if (other.kind == TOK_INT_LIT)
            {
                data.intLit = other.data.intLit;
            }
        }
        return *this;
    }

    Token Token::makeId(const std::string &id, const Span &span)
    {
        Token token(TOK_ID, span);
        if (!id.empty())
        {
            token.data.id = strdup(id.c_str());
            if (!token.data.id)
            {
                throw std::bad_alloc();
            }
        }
        else
        {
            token.data.id = strdup("");
        }
        return token;
    }

    Token Token::makeIntLit(long long value, Base base, const Span &span)
    {
        Token token(TOK_INT_LIT, span);
        token.data.intLit.value = value;
        token.data.intLit.base = base;
        return token;
    }

    Token Token::makeStrLit(const std::string &value, const Span &span)
    {
        Token token(TOK_STR_LIT, span);
        if (!value.empty())
        {
            token.data.strLit = strdup(value.c_str());
            if (!token.data.strLit)
            {
                throw std::bad_alloc();
            }
        }
        else
        {
            token.data.strLit = strdup("");
        }
        return token;
    }

    Token Token::makeFlagShort(const std::string &flag, const Span &span)
    {
        Token token(TOK_FLAG_SHORT, span);
        if (!flag.empty())
        {
            token.data.id = strdup(flag.c_str());
            if (!token.data.id)
            {
                throw std::bad_alloc();
            }
        }
        else
        {
            token.data.id = strdup("");
        }
        return token;
    }

    Token Token::makeFlagLong(const std::string &flag, const Span &span)
    {
        Token token(TOK_FLAG_LONG, span);
        if (!flag.empty())
        {
            token.data.id = strdup(flag.c_str());
            if (!token.data.id)
            {
                throw std::bad_alloc();
            }
        }
        else
        {
            token.data.id = strdup("");
        }
        return token;
    }

    Token Token::makeOptionValue(const std::string &value, const Span &span)
    {
        Token token(TOK_OPTION_VALUE, span);
        if (!value.empty())
        {
            token.data.strLit = strdup(value.c_str());
            if (!token.data.strLit)
            {
                throw std::bad_alloc();
            }
        }
        else
        {
            token.data.strLit = strdup("");
        }
        return token;
    }

    Token Token::makeArgument(const std::string &arg, const Span &span)
    {
        Token token(TOK_ARGUMENT, span);
        if (!arg.empty())
        {
            token.data.strLit = strdup(arg.c_str());
            if (!token.data.strLit)
            {
                throw std::bad_alloc();
            }
        }
        else
        {
            token.data.strLit = strdup("");
        }
        return token;
    }

    Token Token::makeCommand(const std::string &cmd, const Span &span)
    {
        Token token(TOK_COMMAND, span);
        if (!cmd.empty())
        {
            token.data.id = strdup(cmd.c_str());
            if (!token.data.id)
            {
                // Handle memory allocation failure
                throw std::bad_alloc();
            }
        }
        else
        {
            token.data.id = strdup("");
        }
        return token;
    }

    Token Token::makeFloatLit(double value, bool hasExponent, const Span &span)
    {
        Token token(TOK_FLOAT_LIT, span);
        token.data.floatLit.value = value;
        token.data.floatLit.hasExponent = hasExponent;
        return token;
    }

    TokenKind Token::getKind() const
    {
        return kind;
    }

    const Span &Token::getSpan() const
    {
        return span;
    }

    const char *Token::getId() const
    {
        if (kind == TOK_ID)
        {
            return data.id;
        }
        return NULL;
    }

    long long Token::getIntValue() const
    {
        if (kind == TOK_INT_LIT)
        {
            return data.intLit.value;
        }
        return 0;
    }

    Base Token::getIntBase() const
    {
        if (kind == TOK_INT_LIT)
        {
            return data.intLit.base;
        }
        return DEC;
    }

    const char *Token::getStrLit() const
    {
        if (kind == TOK_STR_LIT)
        {
            return data.strLit;
        }
        return NULL;
    }

    double Token::getFloatValue() const
    {
        if (kind == TOK_FLOAT_LIT)
        {
            return data.floatLit.value;
        }
        return 0.0;
    }

    bool Token::hasFloatExponent() const
    {
        if (kind == TOK_FLOAT_LIT)
        {
            return data.floatLit.hasExponent;
        }
        return false;
    }

    Token::~Token()
    {
        if (kind == TOK_ID || kind == TOK_FLAG_SHORT || kind == TOK_FLAG_LONG || kind == TOK_COMMAND)
        {
            free((void *)data.id);
        }
        else if (kind == TOK_STR_LIT || kind == TOK_OPTION_VALUE || kind == TOK_ARGUMENT)
        {
            free((void *)data.strLit);
        }
    }

    void Token::print(std::ostream &os) const
    {
        switch (kind)
        {
        case TOK_EOF:
            os << "<EOF>";
            break;

        // Keywords
        case TOK_IF:
            os << "if";
            break;
        case TOK_ELSE:
            os << "else";
            break;
        case TOK_FOR:
            os << "for";
            break;
        case TOK_IN:
            os << "in";
            break;
        case TOK_WHILE:
            os << "while";
            break;
        case TOK_BREAK:
            os << "break";
            break;
        case TOK_RETURN:
            os << "return";
            break;
        case TOK_INT:
            os << "int";
            break;
        case TOK_BOOL:
            os << "bool";
            break;
        case TOK_STRING:
            os << "string";
            break;
        case TOK_AND:
            os << "and";
            break;
        case TOK_OR:
            os << "or";
            break;
        case TOK_NOT:
            os << "not";
            break;
        case TOK_TRUE:
            os << "true";
            break;
        case TOK_FALSE:
            os << "false";
            break;

        // Operators
        case TOK_ASSIGN:
            os << "=";
            break;
        case TOK_PLUS:
            os << "+";
            break;
        case TOK_MINUS:
            os << "-";
            break;
        case TOK_DOUBLE_MINUS:
            os << "--";
            break;
        case TOK_TIMES:
            os << "*";
            break;
        case TOK_DIVIDE:
            os << "/";
            break;
        case TOK_MODULO:
            os << "%";
            break;
        case TOK_SHL:
            os << "<<";
            break;
        case TOK_SHR:
            os << ">>";
            break;
        case TOK_LESS:
            os << "<";
            break;
        case TOK_GREATER:
            os << ">";
            break;
        case TOK_LESS_EQ:
            os << "<=";
            break;
        case TOK_GREATER_EQ:
            os << ">=";
            break;
        case TOK_EQ:
            os << "==";
            break;
        case TOK_NOT_EQ:
            os << "!=";
            break;
        // Symbols
        case TOK_LPAREN:
            os << "(";
            break;
        case TOK_RPAREN:
            os << ")";
            break;
        case TOK_LBRACE:
            os << "{";
            break;
        case TOK_RBRACE:
            os << "}";
            break;
        case TOK_LBRACKET:
            os << "[";
            break;
        case TOK_RBRACKET:
            os << "]";
            break;
        case TOK_SEMI:
            os << ";";
            break;
        case TOK_COLON:
            os << ":";
            break;
        case TOK_COMMA:
            os << ",";
            break;
        case TOK_DOT:
            os << ".";
            break;
        // Complex tokens
        case TOK_ID:
            os << data.id;
            break;
        case TOK_STR_LIT:
            os << "\"" << data.strLit << "\"";
            break;
        case TOK_INT_LIT:
            if (data.intLit.base == HEX)
            {
                os << "0x" << std::hex << data.intLit.value << std::dec;
            }
            else if (data.intLit.base == BIN)
            {
                os << "0b";
                // Print binary representation
                for (int i = 63; i >= 0; i--)
                {
                    if ((data.intLit.value >> i) & 1)
                    {
                        // Skip leading zeros
                        while (i >= 0)
                        {
                            os << ((data.intLit.value >> i) & 1);
                            i--;
                        }
                    }
                }
                if (data.intLit.value == 0)
                {
                    os << "0";
                }
            }
            else
            {
                os << data.intLit.value;
            }
            break;
        case TOK_FLOAT_LIT:
            if (data.floatLit.hasExponent)
            {
                // Use scientific notation if it was originally written that way
                os.setf(std::ios::scientific, std::ios::floatfield);
            }
            else
            {
                // Use fixed notation if it was written as a decimal
                os.setf(std::ios::fixed, std::ios::floatfield);
            }
            os << data.floatLit.value;
            break;
        }
    }

    std::ostream &operator<<(std::ostream &os, const Token &token)
    {
        token.print(os);
        return os;
    }

} // namespace lexer