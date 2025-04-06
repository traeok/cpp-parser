#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <ostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <exception>
#include <cctype>       // isdigit, isxdigit, isalpha, etc.
#include <cstring>      // strdup, free
#include <map>          // keywords map in Lexer
#include <climits>      // LLONG_MAX
#include <optional>     // For optional flag values (C++17)
#include <variant>      // For representing different argument types (C++17)
#include <memory>       // For std::shared_ptr

namespace lexer
{
    /**
     * Represents a location within a piece of source code.
     * Includes the filename, line, and column.
     */
    class Location
    {
    public:
        Location() : filename(""), line(1), col(1) {}
        Location(const std::string &filename, size_t line, size_t col)
            : filename(filename), line(line), col(col) {}

        // Getters
        const std::string &getFilename() const { return filename; }
        size_t getLine() const { return line; }
        size_t getCol() const { return col; }

        // Print location to stream
        void print(std::ostream &os) const
        {
            os << (filename.empty() ? "<string>" : filename)
               << " (" << line << ":" << col << ")";
        }

    private:
        std::string filename;
        size_t line;
        size_t col;
    };

    // Stream operator for Location
    inline std::ostream &operator<<(std::ostream &os, const Location &loc)
    {
        loc.print(os);
        return os;
    }

    /**
     * Iterator over a Source object's characters.
     * Keeps track of line and column and can produce a Location.
     */
    class SourceIterator
    {
    public:
        SourceIterator(const std::string &filename, const std::vector<char> &code)
            : code(code), filename(filename), line(1), col(1), pos(0) {}

        // Navigation
        char current() const
        {
            if (pos < code.size())
            {
                return code[pos];
            }
            return '\0';
        }

        char peek() const
        {
            if (pos + 1 < code.size())
            {
                return code[pos + 1];
            }
            return '\0';
        }

        void next()
        {
            if (pos < code.size())
            {
                if (code[pos] == '\n')
                {
                    line++;
                    col = 1;
                }
                else if (code[pos] == '\t')
                {
                    // Assuming tab width of 4, adjust if needed
                    col += 4;
                }
                else
                {
                    col++;
                }
                pos++;
            }
        }

        void next2()
        {
            next();
            next();
        }

        // State access
        bool hasMore() const { return pos < code.size(); }
        size_t position() const { return pos; }
        Location getLocation() const { return Location(filename, line, col); }

    private:
        const std::vector<char> &code;
        std::string filename;
        size_t line;
        size_t col;
        size_t pos;
    };

    /**
     * Represents a piece of source code.
     * Includes the filename (if any) and the raw source code.
     */
    class Source
    {
    public:
        Source() {}

        // Factory Methods
        static Source fromFile(const std::string &filename)
        {
            std::ifstream file(filename.c_str());
            if (!file.is_open())
            {
                throw std::runtime_error("Could not open file: " + filename);
            }

            // Read the whole file efficiently
            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            std::vector<char> buffer(size);
            file.seekg(0);
            file.read(buffer.data(), size);

            if (!file) {
                 throw std::runtime_error("Error reading file: " + filename);
            }


            return Source(filename, buffer);
        }

        static Source fromString(const std::string &codeStr, const std::string &filename = "<string>")
        {
            std::vector<char> chars(codeStr.begin(), codeStr.end());
            return Source(filename, chars);
        }

        // Getters
        const std::string &getFilename() const { return filename; }
        const std::vector<char> &getCode() const { return code; }

        // Create an iterator
        SourceIterator getIterator() const { return SourceIterator(filename, code); }

    private:
        // Private constructor used by factory methods
        Source(const std::string &filename, const std::vector<char> &code)
            : filename(filename), code(code) {}

        std::string filename;
        std::vector<char> code;
    };


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
        LexError(const Location &loc, LexErrorKind kind)
            : loc(loc), kind(kind) {}

        // Factory methods for specific errors
        static LexError invalidChar(const Location &loc)
        {
            return LexError(loc, INVALID_CHAR);
        }
        static LexError unclosedString(const Location &loc)
        {
            return LexError(loc, UNCLOSED_STRING);
        }
        static LexError unknownEscape(const Location &loc)
        {
            return LexError(loc, UNKNOWN_ESCAPE);
        }
        static LexError intOutOfRange(const Location &loc)
        {
            return LexError(loc, INT_OUT_OF_RANGE);
        }
        static LexError incompleteInt(const Location &loc)
        {
            return LexError(loc, INCOMPLETE_INT);
        }
        static LexError floatOutOfRange(const Location &loc)
        {
            return LexError(loc, FLOAT_OUT_OF_RANGE);
        }
        static LexError invalidFloat(const Location &loc)
        {
            return LexError(loc, INVALID_FLOAT);
        }

        // Accessors
        const Location &getLocation() const { return loc; }
        LexErrorKind getKind() const { return kind; }

        // Override std::exception::what()
        virtual const char *what() const throw()
        {
            // Generate the message string on demand and cache it
            if (message.empty())
            {
                message = toString();
            }
            return message.c_str();
        }

        // Helper to generate the error message string
        std::string toString() const
        {
            std::ostringstream ss;
            ss << loc << ": "; // Use the overloaded << for Location

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
                // Adjusted message for clarity on 64-bit range
                ss << "integer literal out of 64-bit range";
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
            default: // Should not happen
                 ss << "unknown lexer error";
                 break;
            }

            return ss.str();
        }

    private:
        Location loc;
        LexErrorKind kind;
        // Use mutable to allow modification within a const method (what())
        mutable std::string message;
    };

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

        // CLI-specific tokens (Optional - consider if these are core or should be separate)
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
     * Note: Direct use of union with non-POD types like char* requires careful manual memory management.
     * Consider std::variant (C++17) or a struct with optional fields for a more modern/safer approach.
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

        // Using char* requires manual memory management (strdup/free)
        char *strLit; // Used for TOK_STR_LIT, TOK_OPTION_VALUE, TOK_ARGUMENT
        char *id;     // Used for TOK_ID, TOK_FLAG_SHORT, TOK_FLAG_LONG, TOK_COMMAND

        // Default constructor initializes one member (important for unions)
        TokenData() : strLit(nullptr) {}
        // Need a destructor to clean up potentially allocated memory, but
        // managing this within a union is tricky and depends on knowing the active member.
        // The Token class's destructor will handle this based on the 'kind'.
        // ~TokenData() {} // Cannot have non-trivial destructor in union before C++11 without care
    };

    /**
     * Token class representing a lexical token
     */
    class Token
    {
    public:
        // Default constructor
        Token() : kind(TOK_EOF), span() { data.strLit = nullptr; } // Ensure data is initialized

        // Constructor for simple tokens
        Token(TokenKind kind, const Span &span)
            : kind(kind), span(span) { data.strLit = nullptr; } // Ensure data is initialized

        // Copy constructor - performs deep copy for string/id types
        Token(const Token &other)
            : kind(other.kind), span(other.span)
        {
            copyData(other);
        }

        // Assignment operator - handles self-assignment and deep copy
        Token &operator=(const Token &other)
        {
            if (this != &other)
            {
                // 1. Deallocate existing resources
                freeData();
                // 2. Copy data members
                kind = other.kind;
                span = other.span;
                // 3. Copy resources (deep copy)
                copyData(other);
            }
            return *this;
        }

        // Move constructor (C++11 onwards)
        Token(Token &&other) noexcept
            : kind(other.kind), span(other.span), data(other.data)
        {
            // After moving, the source object should be in a valid state
            // (e.g., null pointers) so its destructor doesn't free the moved resource.
            other.kind = TOK_EOF; // Or some other default state
            other.data.strLit = nullptr; // Prevent double-free
        }

        // Move assignment operator (C++11 onwards)
        Token &operator=(Token &&other) noexcept
        {
             if (this != &other)
             {
                // 1. Deallocate existing resources
                freeData();
                // 2. Transfer ownership of resources
                kind = other.kind;
                span = other.span;
                data = other.data; // Shallow copy of union is okay for move
                // 3. Leave the source object in a valid state
                other.kind = TOK_EOF;
                other.data.strLit = nullptr;
             }
             return *this;
        }


        // Destructor - frees memory allocated for id/strLit
        ~Token()
        {
            freeData();
        }

        // Static factory methods for complex tokens
        static Token makeId(const std::string &id_str, const Span &span)
        {
            Token token(TOK_ID, span);
            token.data.id = strdup(id_str.c_str());
            if (!token.data.id) { throw std::bad_alloc(); }
            return token;
        }

        static Token makeIntLit(long long value, Base base, const Span &span)
        {
            Token token(TOK_INT_LIT, span);
            token.data.intLit.value = value;
            token.data.intLit.base = base;
            return token;
        }

        static Token makeFloatLit(double value, bool hasExponent, const Span &span)
        {
            Token token(TOK_FLOAT_LIT, span);
            token.data.floatLit.value = value;
            token.data.floatLit.hasExponent = hasExponent;
            return token;
        }

        static Token makeStrLit(const std::string &str, const Span &span)
        {
            Token token(TOK_STR_LIT, span);
            token.data.strLit = strdup(str.c_str());
            if (!token.data.strLit) { throw std::bad_alloc(); }
            return token;
        }

        // Static factory methods for CLI tokens
        static Token makeFlagShort(const std::string &flag, const Span &span)
        {
            Token token(TOK_FLAG_SHORT, span);
            token.data.id = strdup(flag.c_str());
            if (!token.data.id) { throw std::bad_alloc(); }
            return token;
        }

        static Token makeFlagLong(const std::string &flag, const Span &span)
        {
            Token token(TOK_FLAG_LONG, span);
            token.data.id = strdup(flag.c_str());
            if (!token.data.id) { throw std::bad_alloc(); }
            return token;
        }

        static Token makeOptionValue(const std::string &value, const Span &span)
        {
            Token token(TOK_OPTION_VALUE, span);
            token.data.strLit = strdup(value.c_str());
            if (!token.data.strLit) { throw std::bad_alloc(); }
            return token;
        }

        static Token makeArgument(const std::string &arg, const Span &span)
        {
            Token token(TOK_ARGUMENT, span);
            token.data.strLit = strdup(arg.c_str());
            if (!token.data.strLit) { throw std::bad_alloc(); }
            return token;
        }

        static Token makeCommand(const std::string &cmd, const Span &span)
        {
            Token token(TOK_COMMAND, span);
            token.data.id = strdup(cmd.c_str());
            if (!token.data.id) { throw std::bad_alloc(); }
            return token;
        }

        // Accessors
        TokenKind getKind() const { return kind; }
        const Span &getSpan() const { return span; }

        // Getter methods for complex token data (with safety checks)
        const char *getId() const
        {
            if (kind == TOK_ID || kind == TOK_FLAG_SHORT || kind == TOK_FLAG_LONG || kind == TOK_COMMAND)
            {
                return data.id;
            }
            // Consider throwing an exception or returning a specific value for incorrect access
            return nullptr; // Or return "" ?
        }

        long long getIntValue() const
        {
            if (kind == TOK_INT_LIT)
            {
                return data.intLit.value;
            }
             // Consider throwing or returning a sentinel
            return 0;
        }

        Base getIntBase() const
        {
            if (kind == TOK_INT_LIT)
            {
                return data.intLit.base;
            }
            return DEC; // Default base
        }

        double getFloatValue() const
        {
            if (kind == TOK_FLOAT_LIT)
            {
                return data.floatLit.value;
            }
            // Consider throwing or returning a sentinel
            return 0.0;
        }

        bool hasFloatExponent() const
        {
            if (kind == TOK_FLOAT_LIT)
            {
                return data.floatLit.hasExponent;
            }
            return false;
        }

        const char *getStrLit() const
        {
            if (kind == TOK_STR_LIT || kind == TOK_OPTION_VALUE || kind == TOK_ARGUMENT)
            {
                return data.strLit;
            }
            // Consider throwing or returning a sentinel
            return nullptr; // Or return "" ?
        }

        // Print token to stream
        void print(std::ostream &os) const
        {
             // Use a static map for keyword/symbol lookup for efficiency
            static const std::map<TokenKind, std::string> simpleTokens = {
                {TOK_EOF, "<EOF>"},
                {TOK_IF, "if"}, {TOK_ELSE, "else"}, {TOK_FOR, "for"}, {TOK_IN, "in"},
                {TOK_WHILE, "while"}, {TOK_BREAK, "break"}, {TOK_RETURN, "return"},
                {TOK_INT, "int"}, {TOK_BOOL, "bool"}, {TOK_STRING, "string"},
                {TOK_AND, "and"}, {TOK_OR, "or"}, {TOK_NOT, "not"},
                {TOK_TRUE, "true"}, {TOK_FALSE, "false"},
                {TOK_ASSIGN, "="}, {TOK_PLUS, "+"}, {TOK_MINUS, "-"}, {TOK_DOUBLE_MINUS, "--"},
                {TOK_TIMES, "*"}, {TOK_DIVIDE, "/"}, {TOK_MODULO, "%"},
                {TOK_SHL, "<<"}, {TOK_SHR, ">>"}, {TOK_LESS, "<"}, {TOK_GREATER, ">"},
                {TOK_LESS_EQ, "<="}, {TOK_GREATER_EQ, ">="}, {TOK_EQ, "=="}, {TOK_NOT_EQ, "!="},
                {TOK_LPAREN, "("}, {TOK_RPAREN, ")"}, {TOK_LBRACE, "{"},
                {TOK_RBRACE, "}"}, {TOK_LBRACKET, "["}, {TOK_RBRACKET, "]"},
                {TOK_SEMI, ";"}, {TOK_COLON, ":"}, {TOK_COMMA, ","}, {TOK_DOT, "."}
            };

            auto it = simpleTokens.find(kind);
            if (it != simpleTokens.end()) {
                os << it->second;
                return;
            }

            // Handle complex tokens
            switch (kind)
            {
            case TOK_ID:
            case TOK_FLAG_SHORT:
            case TOK_FLAG_LONG:
            case TOK_COMMAND:
                os << (data.id ? data.id : "<null_id>"); // Safety check
                break;
            case TOK_STR_LIT:
            case TOK_OPTION_VALUE:
            case TOK_ARGUMENT:
                 os << "\"" << (data.strLit ? data.strLit : "<null_str>") << "\""; // Safety check & add quotes
                break;
            case TOK_INT_LIT:
                if (data.intLit.base == HEX)
                {
                    // Store original flags
                    std::ios_base::fmtflags original_flags = os.flags();
                    os << "0x" << std::hex << data.intLit.value;
                    // Restore original flags (especially std::dec)
                    os.flags(original_flags);
                }
                else if (data.intLit.base == BIN)
                {
                    os << "0b";
                    if (data.intLit.value == 0) {
                        os << "0";
                    } else {
                        // Find the most significant bit
                        int i = 63;
                        while (i >= 0 && !((data.intLit.value >> i) & 1)) {
                             i--;
                        }
                        // Print bits from MSB down to LSB
                        while (i >= 0) {
                            os << ((data.intLit.value >> i) & 1);
                            i--;
                        }
                    }
                }
                else // DEC
                {
                    os << data.intLit.value;
                }
                break;
            case TOK_FLOAT_LIT:
                 {
                    // Store original flags and precision
                    std::ios_base::fmtflags original_flags = os.flags();
                    std::streamsize original_precision = os.precision();

                    if (data.floatLit.hasExponent)
                    {
                        os.setf(std::ios::scientific, std::ios::floatfield);
                    }
                    else
                    {
                        os.setf(std::ios::fixed, std::ios::floatfield);
                        // You might want to set a default precision for fixed floats
                        // os.precision(6); // Example: Set precision to 6 decimal places
                    }
                    os << data.floatLit.value;

                    // Restore original flags and precision
                    os.flags(original_flags);
                    os.precision(original_precision);
                 }
                break;
            // Note: TOK_EOF is handled by the map lookup above
            default: // Should not happen if all kinds are covered
                os << "<Unknown TokenKind: " << static_cast<int>(kind) << ">";
                break;
            }
        }

    private:
        TokenKind kind;
        Span span;
        TokenData data; // The union holding complex data

        // Helper to free dynamically allocated string data
        void freeData() {
            if (kind == TOK_ID || kind == TOK_FLAG_SHORT || kind == TOK_FLAG_LONG || kind == TOK_COMMAND) {
                free(data.id);
                data.id = nullptr; // Avoid dangling pointer issues
            }
            else if (kind == TOK_STR_LIT || kind == TOK_OPTION_VALUE || kind == TOK_ARGUMENT) {
                free(data.strLit);
                data.strLit = nullptr; // Avoid dangling pointer issues
            }
             // No need to free for other types (intLit, floatLit)
        }

         // Helper to copy data, performing deep copy for strings
        void copyData(const Token& other) {
             if (other.kind == TOK_ID || other.kind == TOK_FLAG_SHORT || other.kind == TOK_FLAG_LONG || other.kind == TOK_COMMAND) {
                 data.id = other.data.id ? strdup(other.data.id) : nullptr;
                 if (other.data.id && !data.id) { throw std::bad_alloc(); } // Check strdup success
             }
             else if (other.kind == TOK_STR_LIT || other.kind == TOK_OPTION_VALUE || other.kind == TOK_ARGUMENT) {
                 data.strLit = other.data.strLit ? strdup(other.data.strLit) : nullptr;
                 if (other.data.strLit && !data.strLit) { throw std::bad_alloc(); } // Check strdup success
             }
             else if (other.kind == TOK_INT_LIT) {
                 data.intLit = other.data.intLit;
             }
             else if (other.kind == TOK_FLOAT_LIT) {
                 data.floatLit = other.data.floatLit;
             }
             else {
                 // For simple token kinds, no data needs copying, but ensure union is initialized safely.
                 // Explicitly setting a member helps.
                 data.strLit = nullptr;
             }
         }
    };

    // Stream operator for Token
    inline std::ostream &operator<<(std::ostream &os, const Token &token)
    {
        token.print(os);
        return os;
    }

    /**
     * Class for lexical analysis (tokenization)
     */
    class Lexer
    {
    public:
        /**
         * Primary static function to tokenize source code.
         * Takes a Source object and returns a vector of Tokens.
         */
        static std::vector<Token> tokenize(const Source &source)
        {
            Lexer lexer(source);
            lexer.lexAll();
            return std::move(lexer.tokens); // Use move for efficiency
        }

    private:
        // Private constructor - only used internally by the static tokenize method
        Lexer(const Source &source)
            : iter(source.getIterator())
        {
            // Reserve some space potentially? Might improve performance slightly.
            // tokens.reserve(source.getCode().size() / 5); // Rough estimate
        }

        // Helper functions for character classification (static, could be file-scope static)
        static bool isDecDigitStart(char c) { return std::isdigit(c); }
        static bool isHexDigitStart(char c) { return std::isxdigit(c); }
        static bool isBinDigitStart(char c) { return c == '0' || c == '1'; }
        static bool isIdentStart(char c) { return std::isalpha(c) || c == '$' || c == '_'; }

        // Note: Renamed isDigitCont to isDigitOrUnderscore
        static bool isDigitOrUnderscore(char c, Base base)
        {
            if (c == '_') return true;
            switch (base)
            {
            case DEC: return isDecDigitStart(c);
            case HEX: return isHexDigitStart(c);
            case BIN: return isBinDigitStart(c);
            default: return false; // Should not happen
            }
        }

        static bool isIdentCont(char c)
        {
            // An identifier continuation character can be a start character or a digit.
            return isIdentStart(c) || std::isdigit(c);
        }

        // Core lexing driver function
        void lexAll()
        {
            tokens.clear();
            while (true)
            {
                eatWhitespaceAndComments();
                Token token = nextToken();
                tokens.push_back(std::move(token)); // Move token into vector
                if (tokens.back().getKind() == TOK_EOF)
                {
                    break;
                }
            }
        }

        // Skips over whitespace and single-line comments
        void eatWhitespaceAndComments()
        {
            while (true)
            {
                switch (current())
                {
                // Whitespace
                case ' ':
                case '\t':
                case '\n':
                case '\r': // Handle carriage return as well
                    next();
                    break;

                // Comments (single line //)
                case '/':
                    if (peek() == '/')
                    {
                        next2(); // Consume //
                        // Consume until newline or EOF
                        while (current() != '\n' && current() != '\0')
                        {
                            next();
                        }
                        // No need to consume the newline here, the outer loop handles it
                    }
                    else
                    {
                        // Not a comment, just a slash (handled by nextToken)
                        return;
                    }
                    break;

                // End of whitespace/comment section
                default:
                    return;
                }
            }
        }

        // Lexes the next token from the input stream
        Token nextToken()
        {
            size_t start = iter.position();

            switch (char c = current()) // Use init-statement (C++17)
            {
            // EOF
            case '\0':
                return Token(TOK_EOF, Span(start, start));

            // Single-character tokens that are unambiguous
            case '+': next(); return Token(TOK_PLUS, Span(start, iter.position()));
            case '*': next(); return Token(TOK_TIMES, Span(start, iter.position()));
            case '%': next(); return Token(TOK_MODULO, Span(start, iter.position()));
            case '(': next(); return Token(TOK_LPAREN, Span(start, iter.position()));
            case ')': next(); return Token(TOK_RPAREN, Span(start, iter.position()));
            case '{': next(); return Token(TOK_LBRACE, Span(start, iter.position()));
            case '}': next(); return Token(TOK_RBRACE, Span(start, iter.position()));
            case '[': next(); return Token(TOK_LBRACKET, Span(start, iter.position()));
            case ']': next(); return Token(TOK_RBRACKET, Span(start, iter.position()));
            case ';': next(); return Token(TOK_SEMI, Span(start, iter.position()));
            case ':': next(); return Token(TOK_COLON, Span(start, iter.position()));
            case ',': next(); return Token(TOK_COMMA, Span(start, iter.position()));
            case '.': next(); return Token(TOK_DOT, Span(start, iter.position()));

            // Potentially multi-character tokens
            case '-':
                next();
                if (current() == '-')
                {
                    next(); // Consume second '-'
                    // Check if it's part of a long flag (e.g., --help)
                    if (isIdentStart(current())) {
                         return lexLongFlag();
                    }
                    // Otherwise, it could be just '--' or an error depending on context
                    // For now, let's assume TOK_DOUBLE_MINUS if not followed by identifier start.
                     return Token(TOK_DOUBLE_MINUS, Span(start, iter.position()));
                }
                 // Check if it's a short flag (e.g., -h)
                 else if (isIdentStart(current()) || isDecDigitStart(current())) {
                      return lexShortFlag();
                 }
                 // Regular minus operator
                return Token(TOK_MINUS, Span(start, iter.position()));

            case '/': // Division or start of comment (comment handled by eatWhitespace)
                next();
                // If we reach here, it must be division, as comments were eaten
                return Token(TOK_DIVIDE, Span(start, iter.position()));

            case '<':
                next();
                if (current() == '<') { next(); return Token(TOK_SHL, Span(start, iter.position())); }
                if (current() == '=') { next(); return Token(TOK_LESS_EQ, Span(start, iter.position())); }
                return Token(TOK_LESS, Span(start, iter.position()));

            case '>':
                next();
                if (current() == '>') { next(); return Token(TOK_SHR, Span(start, iter.position())); }
                if (current() == '=') { next(); return Token(TOK_GREATER_EQ, Span(start, iter.position())); }
                return Token(TOK_GREATER, Span(start, iter.position()));

            case '=':
                next();
                if (current() == '=') { next(); return Token(TOK_EQ, Span(start, iter.position())); }
                return Token(TOK_ASSIGN, Span(start, iter.position()));

            case '!':
                next();
                if (current() == '=') { next(); return Token(TOK_NOT_EQ, Span(start, iter.position())); }
                // '!' by itself could be TOK_NOT if supported as a prefix operator
                // Or it could be an error if not part of != or a prefix ! operator
                // Let's assume it's TOK_NOT for now, like the keyword.
                 return Token(TOK_NOT, Span(start, iter.position()));
                 // Alternatively, throw: throw LexError::invalidChar(iter.getLocation());

            // String literals
            case '"':
                return lexString();

            // Numbers (Integers or Floats)
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                return lexNumber(); // Handles int/float, prefixes 0x, 0b

            // Identifiers or Keywords
            default:
                if (isIdentStart(c))
                {
                    // Check if it looks like a command (often at the start or after flags)
                    // This logic might be too simple and belong in a parser, not lexer.
                    // Let's stick to lexing identifiers and let parser interpret meaning.
                    return lexIdentifierOrKeyword();
                }
                // Unknown character
                throw LexError::invalidChar(iter.getLocation());
            }
        }

        // Lexes an identifier or a keyword
        Token lexIdentifierOrKeyword()
        {
            size_t start = iter.position();
            std::string id_str;
            id_str += current(); // Add the start character
            next();

            while (isIdentCont(current()))
            {
                id_str += current();
                next();
            }

            // Check if the identifier is a keyword
            // Use a static map for efficient keyword lookup
            static const std::map<std::string, TokenKind> keywords = {
                {"if", TOK_IF}, {"else", TOK_ELSE}, {"for", TOK_FOR}, {"in", TOK_IN},
                {"while", TOK_WHILE}, {"break", TOK_BREAK}, {"return", TOK_RETURN},
                {"int", TOK_INT}, {"bool", TOK_BOOL}, {"string", TOK_STRING},
                {"and", TOK_AND}, {"or", TOK_OR}, {"not", TOK_NOT},
                {"true", TOK_TRUE}, {"false", TOK_FALSE}
                // Add other keywords if any
            };

            auto it = keywords.find(id_str);
            if (it != keywords.end()) {
                // It's a keyword
                return Token(it->second, Span(start, iter.position()));
            }

            // It's an identifier
            return Token::makeId(id_str, Span(start, iter.position()));
            // Consider if identifiers could be CLI commands/arguments based on context?
            // This usually requires parser-level decisions.
            // If strictly lexing, return TOK_ID. Add specific CLI logic if needed.
        }

        // Lexes a string literal enclosed in double quotes
        Token lexString()
        {
            size_t start = iter.position();
            std::string str_value;
            next(); // Consume the opening quote "

            while (true) {
                 char c = current();
                 if (c == '\0') {
                    throw LexError::unclosedString(iter.getLocation()); // Reached EOF
                 }
                 if (c == '\n') {
                      throw LexError::unclosedString(iter.getLocation()); // Strings cannot contain raw newlines
                 }
                 if (c == '"') {
                     break; // End of string
                 }

                 if (c == '\\') { // Escape sequence
                     next(); // Consume backslash
                     switch (char escaped_char = current()) {
                         case 'n': str_value += '\n'; break;
                         case 'r': str_value += '\r'; break;
                         case 't': str_value += '\t'; break;
                         case '\\': str_value += '\\'; break;
                         case '"': str_value += '"'; break;
                         case '0': str_value += '\0'; break; // Null character escape
                         // Add other escapes like \xHH if needed
                         case '\0': // EOF after backslash
                              throw LexError::unclosedString(iter.getLocation());
                         default:
                              // Optional: Allow escaping any character? str_value += escaped_char;
                              throw LexError::unknownEscape(iter.getLocation());
                     }
                 } else {
                     str_value += c;
                 }
                 next(); // Consume character or escaped sequence char
            }

            next(); // Consume the closing quote "
            return Token::makeStrLit(str_value, Span(start, iter.position()));
        }

        // Lexes a number, which can be an integer (dec, hex, bin) or a float
        Token lexNumber()
        {
            size_t start = iter.position();
            Base base = DEC;
            std::string digits;
            bool is_float = false;
            bool has_exponent = false;

            // Check for base prefixes (0x, 0b)
            if (current() == '0')
            {
                char p = peek();
                if (p == 'x' || p == 'X')
                {
                    base = HEX;
                    next2(); // Consume 0x
                    if (!isHexDigitStart(current()))
                    {
                        // Error: 0x must be followed by hex digits
                        throw LexError::incompleteInt(iter.getLocation());
                    }
                }
                else if (p == 'b' || p == 'B')
                {
                    base = BIN;
                    next2(); // Consume 0b
                    if (!isBinDigitStart(current()))
                    {
                         // Error: 0b must be followed by bin digits
                        throw LexError::incompleteInt(iter.getLocation());
                    }
                }
                // If just '0' followed by non-prefix char, it starts as DEC
                 else {
                      digits += '0';
                      next();
                 }
            }

            // Parse the main part of the number (integer part for floats)
            while (isDigitOrUnderscore(current(), base))
            {
                if (current() != '_') { digits += current(); }
                next();
            }

            // Check for float components (only if base is DEC)
            if (base == DEC)
            {
                // Check for decimal point .
                if (current() == '.' && isDecDigitStart(peek()))
                {
                    is_float = true;
                    digits += '.'; // Add the decimal point
                    next();       // Consume .

                    // Parse fractional part
                    while (isDigitOrUnderscore(current(), DEC))
                    {
                        if (current() != '_') { digits += current(); }
                        next();
                    }
                }

                // Check for exponent e/E
                if (current() == 'e' || current() == 'E')
                {
                    // Check if the exponent part is valid (+/-/digit)
                     char next_char = peek();
                    if (isDecDigitStart(next_char) || next_char == '+' || next_char == '-') {
                        is_float = true; // A number with exponent is always float
                        has_exponent = true;
                        digits += current(); // Add 'e' or 'E'
                        next(); // Consume e/E

                        // Add exponent sign if present
                        if (current() == '+' || current() == '-')
                        {
                            digits += current();
                            next();
                        }

                        // Must have at least one digit in exponent
                        if (!isDecDigitStart(current()))
                        { // Check after consuming optional sign
                            throw LexError::incompleteInt(iter.getLocation()); // Or maybe invalidFloat?
                        }

                        // Parse exponent digits
                        while (isDigitOrUnderscore(current(), DEC))
                        {
                            if (current() != '_') { digits += current(); }
                            next();
                        }
                    }
                     // else: 'e' not followed by valid exponent chars, treat as part of identifier/error later? Or stop number here.
                     // Current behavior: Stop number lexing here if 'e' isn't valid exponent start.
                }
            }
            // Cannot have float parts for hex/bin
             else if (current() == '.' || current() == 'e' || current() == 'E') {
                  // Maybe throw error? Or just stop lexing number here.
                   throw LexError::invalidChar(iter.getLocation()); // e.g., 0xFF.0 is invalid
             }

            // If digits string is empty at this point (e.g., after 0x or 0b prefix)
            if (digits.empty() && (base == HEX || base == BIN)) {
                 throw LexError::incompleteInt(iter.getLocation());
            }
            // Special case: if started with 0 but didn't match 0x/0b, and didn't become float.
             if (digits == "0" && base == DEC && !is_float) {
                   // Just the integer 0
                   return Token::makeIntLit(0, DEC, Span(start, iter.position()));
             }
             // Need to handle case where only prefix was consumed e.g. "0x" then EOF
             // Covered by the digits.empty() check above for HEX/BIN

            // Convert the accumulated digits string
            if (is_float)
            {
                try
                {   // Use stod for double conversion
                    std::size_t processed_chars = 0;
                    double value = std::stod(digits, &processed_chars);
                     // Sanity check - ensure stod consumed the whole string we gave it
                     // This might not be strictly necessary if digits construction is correct
                    if (processed_chars != digits.length()) {
                        // This indicates an internal logic error in digits collection
                         throw LexError::invalidFloat(iter.getLocation());
                    }
                    return Token::makeFloatLit(value, has_exponent, Span(start, iter.position()));
                }
                catch (const std::out_of_range &)
                {
                    throw LexError::floatOutOfRange(iter.getLocation());
                }
                catch (const std::invalid_argument &) // Should not happen if digits are collected correctly
                {
                    throw LexError::invalidFloat(iter.getLocation());
                }
            }
            else // It's an integer
            {
                try
                {
                    long long value = 0;
                    // Use stoll with the correct base
                    std::size_t processed_chars = 0;
                    int stoll_base = (base == HEX) ? 16 : (base == BIN) ? 2 : 10;
                    value = std::stoll(digits, &processed_chars, stoll_base);

                     // Sanity check
                    if (processed_chars != digits.length()) {
                         throw LexError::incompleteInt(iter.getLocation()); // Or invalid int?
                    }

                    return Token::makeIntLit(value, base, Span(start, iter.position()));
                }
                catch (const std::out_of_range &) // Catches overflow/underflow
                {
                    throw LexError::intOutOfRange(iter.getLocation());
                }
                 catch (const std::invalid_argument &) // Should not happen if digits ok
                {
                      throw LexError::incompleteInt(iter.getLocation());
                 }
            }
        }

         // Functions for CLI-specific token types (Placeholder implementations)
         // These often depend heavily on the context (e.g., after `--`)
         // which a simple lexer might not track easily.
         // Consider if these should be handled by the parser instead.

         // Example: Lexing a short flag like -v or -fFILENAME
         Token lexShortFlag()
         {
             size_t start = iter.position() - 1; // Start from the '-'
             std::string flag_content;
             // Assume short flag is usually one char, but can sometimes be clustered (-xvf)
             // or have an attached value (-fFILENAME). Simple approach: read alphanumeric sequence.
             while (isIdentCont(current()) || isDecDigitStart(current())) {
                 flag_content += current();
                 next();
             }
             if (flag_content.empty()) {
                 // Just a single '-' was encountered, handled elsewhere? Or error?
                  throw LexError::invalidChar(iter.getLocation()); // Or return TOK_MINUS?
             }
             return Token::makeFlagShort(flag_content, Span(start, iter.position()));
         }

         // Example: Lexing a long flag like --version or --file=myfile
         Token lexLongFlag()
         {
              size_t start = iter.position() - 2; // Start from the '--'
              std::string flag_name;
               // Read the identifier part of the flag
              while (isIdentCont(current())) {
                  flag_name += current();
                  next();
              }
              if (flag_name.empty()) {
                   // Just '--' encountered. This often signals end of options.
                   // Could return a special token, or let parser handle.
                   // For now, treat as error or maybe TOK_DOUBLE_MINUS?
                   throw LexError::invalidChar(iter.getLocation());
              }

              // Check for =value attached to the flag
              if (current() == '=') {
                  next(); // Consume '='
                  // The value part needs lexing too - could be string, number, bareword
                  // This shows complexity - lexer might need modes or parser feedback.
                  // Simplistic: Assume value continues until whitespace. Potential issue with quotes.
                  std::string value_part;
                   size_t value_start = iter.position();
                  while (current() != '\0' && !std::isspace(current())) {
                      value_part += current();
                      next();
                  }
                   // Create a flag token and maybe an option value token?
                   // This structure gets complex quickly. Often '--flag=value' is one token,
                   // or '--flag' and 'value' are separate tokens handled by parser.
                   // Let's just return the flag name token for simplicity here.
                  // A parser would handle the '=' and the value token that follows.
                  return Token::makeFlagLong(flag_name, Span(start, start + 2 + flag_name.length()));
              }

              return Token::makeFlagLong(flag_name, Span(start, iter.position()));
         }

        // Note: lexCLIOptionValue, lexCLICommand, lexCLIArgument would require
        // more context or parser interaction than typically done in a pure lexer.
        // They often overlap with TOK_ID, TOK_STR_LIT, TOK_INT_LIT, etc.
        // We'll omit their specific lexing functions here and rely on the main
        // lexIdentifier/lexString/lexNumber and let the parser interpret them.


        // Character navigation helpers (inline wrappers around iterator)
        char current() const { return iter.current(); }
        char peek() const { return iter.peek(); }
        void next() { iter.next(); }
        void next2() { iter.next2(); }

        // State
        SourceIterator iter;       // Iterator over the source code
        std::vector<Token> tokens; // Vector to store the generated tokens
    };

} // namespace lexer

#endif // LEXER_HPP 