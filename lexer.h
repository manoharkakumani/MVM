#ifndef __LEXER_H__
#define __LEXER_H__

#include "tokens.h"
#include "memory.h"

typedef struct Lexer
{
    size_t len;
    size_t line;
    size_t col;
    size_t indent;
    int currentChar;
    const char *src;
    size_t srcLen;
    const char *token;
} Lexer;

void lexerAdvance(Lexer *lexer);
Token getToken(Lexer *lexer);
Token lexerAdvanceToken(Lexer *lexer, Token token);
Lexer *initLexer(const char *src);
void freeLexer(Lexer *lexer);

#endif