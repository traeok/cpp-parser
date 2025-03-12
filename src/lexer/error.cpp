#include "lexer/error.h"
#include <sstream>

namespace lexer
{

    LexError::LexError(const Location &loc, LexErrorKind kind)
        : loc(loc), kind(kind)
    {
    }

    LexError LexError::invalidChar(const Location &loc)
    {
        return LexError(loc, INVALID_CHAR);
    }

    LexError LexError::unclosedString(const Location &loc)
    {
        return LexError(loc, UNCLOSED_STRING);
    }

    LexError LexError::unknownEscape(const Location &loc)
    {
        return LexError(loc, UNKNOWN_ESCAPE);
    }

    LexError LexError::intOutOfRange(const Location &loc)
    {
        return LexError(loc, INT_OUT_OF_RANGE);
    }

    LexError LexError::incompleteInt(const Location &loc)
    {
        return LexError(loc, INCOMPLETE_INT);
    }

    LexError LexError::floatOutOfRange(const Location &loc)
    {
        return LexError(loc, FLOAT_OUT_OF_RANGE);
    }

    LexError LexError::invalidFloat(const Location &loc)
    {
        return LexError(loc, INVALID_FLOAT);
    }

    const Location &LexError::getLocation() const
    {
        return loc;
    }

    LexErrorKind LexError::getKind() const
    {
        return kind;
    }

    const char *LexError::what() const throw()
    {
        if (message.empty())
        {
            message = toString();
        }
        return message.c_str();
    }

    std::string LexError::toString() const
    {
        std::ostringstream ss;
        ss << loc << ": ";

        switch (kind)
        {
        case INVALID_CHAR:
            ss << "invalid character";
            break;
        case UNCLOSED_STRING:
            ss << "unclosed string literal";
            break;
        case UNKNOWN_ESCAPE:
            ss << "unknown escape character";
            break;
        case INT_OUT_OF_RANGE:
            ss << "integer literal out of 32-bit range";
            break;
        case INCOMPLETE_INT:
            ss << "incomplete integer literal";
            break;
        case FLOAT_OUT_OF_RANGE:
            ss << "floating-point literal out of range";
            break;
        case INVALID_FLOAT:
            ss << "invalid floating-point literal";
            break;
        }

        return ss.str();
    }

} // namespace lexer