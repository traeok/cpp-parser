#include "lexer/source.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace lexer
{

    //
    // Location implementation
    //

    Location::Location() : line(1), col(1)
    {
    }

    Location::Location(const std::string &filename, size_t line, size_t col)
        : filename(filename), line(line), col(col)
    {
    }

    const std::string &Location::getFilename() const
    {
        return filename;
    }

    size_t Location::getLine() const
    {
        return line;
    }

    size_t Location::getCol() const
    {
        return col;
    }

    void Location::print(std::ostream &os) const
    {
        os << filename << " (" << line << ":" << col << ")";
    }

    std::ostream &operator<<(std::ostream &os, const Location &loc)
    {
        loc.print(os);
        return os;
    }

    //
    // SourceIterator implementation
    //

    SourceIterator::SourceIterator(const std::string &filename, const std::vector<char> &code)
        : code(code), filename(filename), line(1), col(1), pos(0)
    {
    }

    char SourceIterator::current() const
    {
        if (pos < code.size())
        {
            return code[pos];
        }
        return '\0';
    }

    char SourceIterator::peek() const
    {
        if (pos + 1 < code.size())
        {
            return code[pos + 1];
        }
        return '\0';
    }

    void SourceIterator::next()
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
                col += 4;
            }
            else
            {
                col++;
            }
            pos++;
        }
    }

    void SourceIterator::next2()
    {
        next();
        next();
    }

    bool SourceIterator::hasMore() const
    {
        return pos < code.size();
    }

    size_t SourceIterator::position() const
    {
        return pos;
    }

    Location SourceIterator::getLocation() const
    {
        return Location(filename, line, col);
    }

    //
    // Source implementation
    //

    Source::Source()
    {
    }

    Source::Source(const std::string &filename, const std::vector<char> &code)
        : filename(filename), code(code)
    {
    }

    Source Source::fromFile(const std::string &filename)
    {
        std::ifstream file(filename.c_str());
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open file: " + filename);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string contents = buffer.str();

        std::vector<char> chars;
        chars.reserve(contents.size());
        for (size_t i = 0; i < contents.size(); i++)
        {
            chars.push_back(contents[i]);
        }

        return Source(filename, chars);
    }

    Source Source::fromString(const std::string &code, const std::string &filename)
    {
        std::vector<char> chars;
        chars.reserve(code.size());
        for (size_t i = 0; i < code.size(); i++)
        {
            chars.push_back(code[i]);
        }

        return Source(filename, chars);
    }

    const std::string &Source::getFilename() const
    {
        return filename;
    }

    const std::vector<char> &Source::getCode() const
    {
        return code;
    }

    SourceIterator Source::getIterator() const
    {
        return SourceIterator(filename, code);
    }

} // namespace lexer