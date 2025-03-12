#include "lexer/lexer.h"
#include <cctype>
#include <stdexcept>
#include <map>
#include <climits> // For LLONG_MAX

namespace lexer
{

    // Helper functions for character classification
    bool Lexer::isDecDigitStart(char c)
    {
        return isdigit(c) != 0;
    }

    bool Lexer::isHexDigitStart(char c)
    {
        return isxdigit(c) != 0;
    }

    bool Lexer::isBinDigitStart(char c)
    {
        return c == '0' || c == '1';
    }

    bool Lexer::isDigitCont(char c, Base base)
    {
        switch (base)
        {
        case DEC:
            return isDecDigitStart(c) || c == '_';
        case HEX:
            return isHexDigitStart(c) || c == '_';
        case BIN:
            return isBinDigitStart(c) || c == '_';
        default:
            return false;
        }
    }

    bool Lexer::isIdentStart(char c)
    {
        return isalpha(c) != 0 || c == '$' || c == '_';
    }

    bool Lexer::isIdentCont(char c)
    {
        return isIdentStart(c) || isdigit(c) != 0;
    }

    // Lexer implementation
    Lexer::Lexer(const Source &source)
        : iter(source.getIterator())
    {
    }

    std::vector<Token> Lexer::tokenize(const Source &source)
    {
        Lexer lexer(source);
        lexer.lexAll();
        return lexer.tokens;
    }

    char Lexer::current() const
    {
        return iter.current();
    }

    char Lexer::peek() const
    {
        return iter.peek();
    }

    void Lexer::next()
    {
        iter.next();
    }

    void Lexer::next2()
    {
        iter.next2();
    }

    void Lexer::lexAll()
    {
        tokens.clear();
        while (true)
        {
            eatWhitespaceAndComments();
            Token token = nextToken();
            tokens.push_back(token);
            if (token.getKind() == TOK_EOF)
            {
                break;
            }
        }
    }

    void Lexer::eatWhitespaceAndComments()
    {
        while (true)
        {
            switch (current())
            {
            // whitespace
            case ' ':
            case '\t':
            case '\n':
                next();
                break;

            // comments
            case '/':
                if (peek() == '/')
                {
                    next2();
                    while (current() != '\n' && current() != '\0')
                    {
                        next();
                    }
                }
                else
                {
                    return;
                }
                break;

            // something that's not whitespace or a comment
            default:
                return;
            }
        }
    }

    Token Lexer::nextToken()
    {
        size_t start = iter.position();

        switch (current())
        {
        // EOF
        case '\0':
            return Token(TOK_EOF, Span(start, start));

        // Single-character tokens
        case '+':
            next();
            return Token(TOK_PLUS, Span(start, iter.position()));
        case '*':
            next();
            return Token(TOK_TIMES, Span(start, iter.position()));
        case '%':
            next();
            return Token(TOK_MODULO, Span(start, iter.position()));
        case '(':
            next();
            return Token(TOK_LPAREN, Span(start, iter.position()));
        case ')':
            next();
            return Token(TOK_RPAREN, Span(start, iter.position()));
        case '{':
            next();
            return Token(TOK_LBRACE, Span(start, iter.position()));
        case '}':
            next();
            return Token(TOK_RBRACE, Span(start, iter.position()));
        case '[':
            next();
            return Token(TOK_LBRACKET, Span(start, iter.position()));
        case ']':
            next();
            return Token(TOK_RBRACKET, Span(start, iter.position()));
        case ';':
            next();
            return Token(TOK_SEMI, Span(start, iter.position()));
        case ':':
            next();
            return Token(TOK_COLON, Span(start, iter.position()));
        case ',':
            next();
            return Token(TOK_COMMA, Span(start, iter.position()));
        case '.':
            next();
            return Token(TOK_DOT, Span(start, iter.position()));

        // Potentially multi-character tokens
        case '-':
            next();
            if (current() == '-')
            {
                next();
                return Token(TOK_DOUBLE_MINUS, Span(start, iter.position()));
            }
            return Token(TOK_MINUS, Span(start, iter.position()));

        case '/':
            next();
            return Token(TOK_DIVIDE, Span(start, iter.position()));

        case '<':
            next();
            if (current() == '<')
            {
                next();
                return Token(TOK_SHL, Span(start, iter.position()));
            }
            else if (current() == '=')
            {
                next();
                return Token(TOK_LESS_EQ, Span(start, iter.position()));
            }
            return Token(TOK_LESS, Span(start, iter.position()));

        case '>':
            next();
            if (current() == '>')
            {
                next();
                return Token(TOK_SHR, Span(start, iter.position()));
            }
            else if (current() == '=')
            {
                next();
                return Token(TOK_GREATER_EQ, Span(start, iter.position()));
            }
            return Token(TOK_GREATER, Span(start, iter.position()));

        case '=':
            next();
            if (current() == '=')
            {
                next();
                return Token(TOK_EQ, Span(start, iter.position()));
            }
            return Token(TOK_ASSIGN, Span(start, iter.position()));

        case '!':
            next();
            if (current() == '=')
            {
                next();
                return Token(TOK_NOT_EQ, Span(start, iter.position()));
            }
            throw LexError::invalidChar(iter.getLocation());

        case '"':
            return lexString();

        case '0':
            if (peek() == 'x' || peek() == 'X' || peek() == 'b' || peek() == 'B')
            {
                return lexInteger();
            }
            // fall through to digit case
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return lexInteger();

        default:
            if (isIdentStart(current()))
            {
                return lexIdentifier();
            }
            throw LexError::invalidChar(iter.getLocation());
        }
    }

    Token Lexer::lexIdentifier()
    {
        size_t start = iter.position();
        std::string id;

        while (isIdentCont(current()))
        {
            id += current();
            next();
        }

        // Check for keywords
        static std::map<std::string, TokenKind> keywords;
        if (keywords.empty()) {
            keywords["if"] = TOK_IF;
            keywords["else"] = TOK_ELSE;
            keywords["for"] = TOK_FOR;
            keywords["in"] = TOK_IN;
            keywords["while"] = TOK_WHILE;
            keywords["break"] = TOK_BREAK;
            keywords["return"] = TOK_RETURN;
            keywords["int"] = TOK_INT;
            keywords["bool"] = TOK_BOOL;
            keywords["string"] = TOK_STRING;
            keywords["and"] = TOK_AND;
            keywords["or"] = TOK_OR;
            keywords["not"] = TOK_NOT;
            keywords["true"] = TOK_TRUE;
            keywords["false"] = TOK_FALSE;
        }

        auto it = keywords.find(id);
        if (it != keywords.end())
        {
            return Token(it->second, Span(start, iter.position()));
        }

        return Token::makeId(id, Span(start, iter.position()));
    }

    Token Lexer::lexString()
    {
        size_t start = iter.position();
        std::string str;

        next(); // Skip opening quote

        while (current() != '"' && current() != '\0')
        {
            if (current() == '\\')
            {
                next();
                switch (current())
                {
                case 'n':
                    str += '\n';
                    break;
                case 'r':
                    str += '\r';
                    break;
                case 't':
                    str += '\t';
                    break;
                case '\\':
                    str += '\\';
                    break;
                case '"':
                    str += '"';
                    break;
                case '0':
                    str += '\0';
                    break;
                default:
                    throw LexError::unknownEscape(iter.getLocation());
                }
            }
            else
            {
                str += current();
            }
            next();
        }

        if (current() == '\0')
        {
            throw LexError::unclosedString(iter.getLocation());
        }

        next(); // Skip closing quote
        return Token::makeStrLit(str, Span(start, iter.position()));
    }

    Token Lexer::lexInteger()
    {
        size_t start = iter.position();
        Base base = DEC;
        std::string digits;
        bool isFloat = false;
        bool hasExponent = false;

        // Check for prefix
        if (current() == '0')
        {
            next();
            if (current() == 'x' || current() == 'X')
            {
                base = HEX;
                next();
                if (!isHexDigitStart(current()))
                {
                    throw LexError::incompleteInt(iter.getLocation());
                }
            }
            else if (current() == 'b' || current() == 'B')
            {
                base = BIN;
                next();
                if (!isBinDigitStart(current()))
                {
                    throw LexError::incompleteInt(iter.getLocation());
                }
            }
            else
            {
                digits = "0";
            }
        }

        // Parse integer part
        while (isDigitCont(current(), base))
        {
            if (current() != '_')
            {
                digits += current();
            }
            next();
        }

        // Check for decimal point
        if (base == DEC && current() == '.')
        {
            if (isDecDigitStart(peek()))
            {
                isFloat = true;
                digits += current(); // add the decimal point
                next();

                // Parse fractional part
                while (isDigitCont(current(), DEC))
                {
                    if (current() != '_')
                    {
                        digits += current();
                    }
                    next();
                }
            }
        }

        // Check for exponent
        if (base == DEC && (current() == 'e' || current() == 'E'))
        {
            char next_char = peek();
            if (isDecDigitStart(next_char) || next_char == '+' || next_char == '-')
            {
                isFloat = true;
                hasExponent = true;
                digits += current(); // add 'e' or 'E'
                next();

                // Add exponent sign if present
                if (current() == '+' || current() == '-')
                {
                    digits += current();
                    next();
                }

                // Must have at least one digit in exponent
                if (!isDecDigitStart(current()))
                {
                    throw LexError::incompleteInt(iter.getLocation());
                }

                // Parse exponent digits
                while (isDigitCont(current(), DEC))
                {
                    if (current() != '_')
                    {
                        digits += current();
                    }
                    next();
                }
            }
        }

        if (isFloat)
        {
            try
            {
                double value = std::stod(digits);
                return Token::makeFloatLit(value, hasExponent, Span(start, iter.position()));
            }
            catch (const std::out_of_range &)
            {
                throw LexError::floatOutOfRange(iter.getLocation());
            }
            catch (const std::invalid_argument &)
            {
                throw LexError::invalidFloat(iter.getLocation());
            }
        }

        // Convert to integer value
        try
        {
            long long value = 0;
            if (base == HEX)
            {
                for (char c : digits)
                {
                    int digit;
                    if (c >= '0' && c <= '9')
                    {
                        digit = c - '0';
                    }
                    else if (c >= 'a' && c <= 'f')
                    {
                        digit = 10 + (c - 'a');
                    }
                    else if (c >= 'A' && c <= 'F')
                    {
                        digit = 10 + (c - 'A');
                    }
                    else
                    {
                        throw LexError::invalidChar(iter.getLocation());
                    }

                    if (value > (LLONG_MAX - digit) / 16)
                    {
                        throw LexError::intOutOfRange(iter.getLocation());
                    }
                    value = value * 16 + digit;
                }
            }
            else if (base == BIN)
            {
                for (char c : digits)
                {
                    int digit = c - '0';
                    if (value > (LLONG_MAX - digit) / 2)
                    {
                        throw LexError::intOutOfRange(iter.getLocation());
                    }
                    value = value * 2 + digit;
                }
            }
            else
            {
                for (char c : digits)
                {
                    int digit = c - '0';
                    if (value > (LLONG_MAX - digit) / 10)
                    {
                        throw LexError::intOutOfRange(iter.getLocation());
                    }
                    value = value * 10 + digit;
                }
            }
            return Token::makeIntLit(value, base, Span(start, iter.position()));
        }
        catch (const std::out_of_range &)
        {
            throw LexError::intOutOfRange(iter.getLocation());
        }
    }

} // namespace lexer