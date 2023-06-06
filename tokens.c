#include "tokens.h"

Token newToken(const char *src, size_t type, size_t len, size_t col, size_t indent, size_t line)
{
    Token tok;
    tok.token = src;
    tok.type = type;
    tok.length = len;
    tok.col = col;
    tok.indent = indent;
    tok.line = line;
    return tok;
}
Keywords keywords[31] = {
    {AND, "and", 3},
    {AS, "as", 2},
    {BREAK, "break", 4},
    {CASE, "case", 4},
    {CLASS, "class", 5},
    {CONTINUE, "continue", 8},
    {CATCH, "catch", 5},
    {DEL, "del", 3},
    {ELSE, "else", 4},
    {ELIF, "elif", 4},
    {FALSE, "False", 5},
    {FINALLY, "final", 5},
    {FOR, "for", 3},
    {FN, "fn", 2},
    {FROM, "from", 4},
    {IF, "if", 2},
    {IN, "in", 2},
    {IS, "is", 2},
    {NOT, "not", 3},
    {NIL, "Nil", 3},
    {OR, "or", 2},
    {PASS, "pass", 4},
    {RAISE, "raise", 5},
    {RET, "return", 6},
    {SUPER, "super", 5},
    {TRY, "try", 3},
    {TRUE, "True", 4},
    {USE, "use", 3},
    {WHILE, "while", 5},
    {XOR, "xor", 3},
    {YIELD, "yield", 5}};

int iskeyword(const char *s, int len)
{
    for (int i = 0; i < 31; i++)
    {
        if ((!memcmp(keywords[i].keyword, s, keywords[i].len)) && (!memcmp(s, keywords[i].keyword, len)))
        {
            return keywords[i].key;
        }
    }
    return 0;
}

void printToken(Token *Token)
{
    printf("%.*s\n", Token->length, Token->token);
}