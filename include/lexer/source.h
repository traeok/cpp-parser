#ifndef SOURCE_H
#define SOURCE_H

#include <string>
#include <vector>
#include <ostream>

namespace lexer
{

    /**
     * Represents a location within a piece of source code.
     * Includes the filename, line, and column.
     */
    class Location
    {
    public:
        Location();
        Location(const std::string &filename, size_t line, size_t col);

        // Getters
        const std::string &getFilename() const;
        size_t getLine() const;
        size_t getCol() const;

        // Print location to stream
        void print(std::ostream &os) const;

    private:
        std::string filename;
        size_t line;
        size_t col;
    };

    // Stream operator for Location
    std::ostream &operator<<(std::ostream &os, const Location &loc);

    /**
     * Iterator over a Source object's characters.
     * Keeps track of line and column and can produce a Location.
     */
    class SourceIterator
    {
    public:
        SourceIterator(const std::string &filename, const std::vector<char> &code);

        // Navigation
        char current() const;
        char peek() const;
        void next();
        void next2();

        // State access
        bool hasMore() const;
        size_t position() const;
        Location getLocation() const;

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
        Source();

        // Constructors
        static Source fromFile(const std::string &filename);
        static Source fromString(const std::string &code, const std::string &filename);

        // Getters
        const std::string &getFilename() const;
        const std::vector<char> &getCode() const;

        // Create an iterator
        SourceIterator getIterator() const;

    private:
        Source(const std::string &filename, const std::vector<char> &code);

        std::string filename;
        std::vector<char> code;
    };

} // namespace lexer

#endif // SOURCE_H