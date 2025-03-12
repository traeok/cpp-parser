static LexError intOutOfRange(const Location &loc)
{
    return LexError("Integer literal out of range", loc);
}

static LexError floatOutOfRange(const Location &loc)
{
    return LexError("Floating point literal out of range", loc);
}

static LexError invalidFloat(const Location &loc)
{
    return LexError("Invalid floating point literal", loc);
}