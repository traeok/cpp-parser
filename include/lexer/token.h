#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <vector>
#include <ostream>

namespace lexer
{

    /**
     * Enumeration for integer literal base
     */
    enum Base
    {
        DEC = 10,
        BIN = 2,
        HEX = 16
    };

    /**
     * Token kind enumeration
     */
    enum TokenKind
    {
        // "end-of-file" - used as a sentinel value at the end of the token stream.
        TOK_EOF,

        // keywords
        TOK_IF,     // if
        TOK_ELSE,   // else
        TOK_FOR,    // for
        TOK_IN,     // in
        TOK_WHILE,  // while
        TOK_BREAK,  // break
        TOK_RETURN, // return
        TOK_INT,    // int
        TOK_BOOL,   // bool
        TOK_STRING, // string
        TOK_AND,    // and
        TOK_OR,     // or
        TOK_NOT,    // not
        TOK_TRUE,   // true
        TOK_FALSE,  // false

        // operators
        TOK_ASSIGN,       // =
        TOK_PLUS,         // +
        TOK_MINUS,        // -
        TOK_DOUBLE_MINUS, // --
        TOK_TIMES,        // *
        TOK_DIVIDE,       // /
        TOK_MODULO,       // %
        TOK_SHL,          // <<
        TOK_SHR,          // >>
        TOK_LESS,         // <
        TOK_GREATER,      // >
        TOK_LESS_EQ,      // <=
        TOK_GREATER_EQ,   // >=
        TOK_EQ,           // ==
        TOK_NOT_EQ,       // !=

        // other symbols
        TOK_LPAREN,   // (
        TOK_RPAREN,   // )
        TOK_LBRACE,   // {
        TOK_RBRACE,   // }
        TOK_LBRACKET, // [
        TOK_RBRACKET, // ]
        TOK_SEMI,     // ;
        TOK_COLON,    // :
        TOK_COMMA,    // ,
        TOK_DOT,      // .

        // CLI-specific tokens
        TOK_FLAG_SHORT,   // -h, -v (single character flags)
        TOK_FLAG_LONG,    // --help, --version (word flags)
        TOK_OPTION_VALUE, // value part in --key=value or --key value
        TOK_ARGUMENT,     // positional arguments
        TOK_COMMAND,      // subcommands (e.g., git commit, where 'commit' is a command)

        // more complex tokens - these require additional data
        TOK_ID,        // identifiers (names of variables, functions etc)
        TOK_INT_LIT,   // integer value + base it was written in
        TOK_FLOAT_LIT, // floating point numbers
        TOK_STR_LIT    // quoted strings
    };

    /**
     * Span representing a range in the source code
     */
    struct Span
    {
        size_t start;
        size_t end;

        Span() : start(0), end(0) {}
        Span(size_t s, size_t e) : start(s), end(e) {}
    };

    /**
     * Token data union for storing complex token data
     */
    union TokenData
    {
        struct
        {
            long long value;
            Base base;
        } intLit;

        struct
        {
            double value;
            bool hasExponent; // true if number was written in scientific notation
        } floatLit;

        // Note: Using char* for string literals and identifiers is not ideal
        // but pre-C++11 doesn't have good ways to handle this without custom allocations
        // In a real implementation, we'd need to handle the memory management properly
        const char *strLit;
        const char *id;

        TokenData() : strLit(NULL) {}
    };

    /**
     * Token class representing a lexical token
     */
    class Token
    {
    public:
        Token();
        Token(TokenKind kind, const Span &span);

        // Add copy constructor and assignment operator
        Token(const Token &other);
        Token &operator=(const Token &other);

        // Static factory methods for complex tokens
        static Token makeId(const std::string &id, const Span &span);
        static Token makeIntLit(long long value, Base base, const Span &span);
        static Token makeFloatLit(double value, bool hasExponent, const Span &span);
        static Token makeStrLit(const std::string &value, const Span &span);

        // Static factory methods for CLI tokens
        static Token makeFlagShort(const std::string &flag, const Span &span);
        static Token makeFlagLong(const std::string &flag, const Span &span);
        static Token makeOptionValue(const std::string &value, const Span &span);
        static Token makeArgument(const std::string &arg, const Span &span);
        static Token makeCommand(const std::string &cmd, const Span &span);

        // Accessors
        TokenKind getKind() const;
        const Span &getSpan() const;

        // Getter methods for complex token data
        const char *getId() const;
        long long getIntValue() const;
        Base getIntBase() const;
        double getFloatValue() const;
        bool hasFloatExponent() const;
        const char *getStrLit() const;

        // Print token to stream
        void print(std::ostream &os) const;

        // Destructor to handle string memory
        ~Token();

    private:
        TokenKind kind;
        Span span;
        TokenData data;
    };

    // Stream operator
    std::ostream &operator<<(std::ostream &os, const Token &token);

} // namespace lexer

#endif // TOKEN_H