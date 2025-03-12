#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include "lexer/token.h"
#include "lexer/source.h"
#include "lexer/error.h"

namespace lexer
{

    /**
     * Class for lexical analysis (tokenization)
     */
    class Lexer
    {
    public:
        // Primary function to tokenize source code
        static std::vector<Token> tokenize(const Source &source);

    private:
        // Private constructor - only used internally
        Lexer(const Source &source);

        // Helper functions for character classification
        static bool isDecDigitStart(char c);
        static bool isHexDigitStart(char c);
        static bool isBinDigitStart(char c);
        static bool isDigitCont(char c, Base base);
        static bool isIdentStart(char c);
        static bool isIdentCont(char c);

        // Core lexing functions
        void lexAll();
        void eatWhitespaceAndComments();
        Token nextToken();

        // Functions for specific token types
        Token lexIdentifier();
        Token lexString();
        Token lexInteger();

        // Functions for CLI-specific token types
        Token lexShortFlag();
        Token lexLongFlag();
        Token lexCLIOptionValue();
        Token lexCLICommand();
        Token lexCLIArgument();

        // Character navigation helpers
        char current() const;
        char peek() const;
        void next();
        void next2();

        // State
        SourceIterator iter;
        std::vector<Token> tokens;
    };

} // namespace lexer

#endif // LEXER_H