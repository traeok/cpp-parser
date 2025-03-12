#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <exception>
#include "lexer/source.h"

namespace lexer
{

    /**
     * Types of lexer errors
     */
    enum LexErrorKind
    {
        INVALID_CHAR,
        UNCLOSED_STRING,
        UNKNOWN_ESCAPE,
        INT_OUT_OF_RANGE,
        INCOMPLETE_INT,
        FLOAT_OUT_OF_RANGE,
        INVALID_FLOAT
    };

    /**
     * Lexer error class
     */
    class LexError : public std::exception
    {
    public:
        LexError(const Location &loc, LexErrorKind kind);

        // Factory methods for specific errors
        static LexError invalidChar(const Location &loc);
        static LexError unclosedString(const Location &loc);
        static LexError unknownEscape(const Location &loc);
        static LexError intOutOfRange(const Location &loc);
        static LexError incompleteInt(const Location &loc);
        static LexError floatOutOfRange(const Location &loc);
        static LexError invalidFloat(const Location &loc);

        // Accessors
        const Location &getLocation() const;
        LexErrorKind getKind() const;

        // Print error to string
        virtual const char *what() const throw();
        std::string toString() const;

    private:
        Location loc;
        LexErrorKind kind;
        mutable std::string message;
    };

} // namespace lexer

#endif // ERROR_H