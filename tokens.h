#ifndef __TOKEN_H__

#define __TOKEN_H__

#include "common.h"

typedef enum
{
    START = 0,
    END,
    ERROR,

    // tokens

    ARROW,
    AMPER,
    AT,
    BSLASH,
    CAP,
    COLON,
    COMMA,
    DOLLAR,
    DOT,
    DOUBLE,
    DSLASH,
    DSTAR,
    DLESS,
    DGREATER,
    DEQUAL,
    EAMPER,
    ECAP,
    EDGREATER,
    EDLESS,
    EDSLASH,
    EDSTAR,
    EGREATER,
    ELESS,
    EMINUS,
    EPERCENT,
    EPLUS,
    EQUAL,
    ESTAR,
    ESLASH,
    EVBAR,
    EXCMARK,
    GREATER,
    INT,
    LBRACE,
    LESS,
    LPAR,
    LSQB,
    MINUS,
    NAME,
    NEQUAL,
    NEWLINE,
    PERCENT,
    PITHRU,
    PLUS,
    QMARK,
    RBRACE,
    RPAR,
    RSQB,
    SEMI,
    SLASH,
    STAR,
    STRING,
    TILD,
    UMINUS, // -@
    UPLUS,  // +@
    VBAR,

    // keywords
    AND,
    AS,
    BREAK,
    CASE,
    CLASS,
    CONTINUE,
    CATCH,
    DEL,
    ELSE,
    ELIF,
    FALSE,
    FINALLY,
    FOR,
    FN,
    FROM,
    IF,
    IN,
    IS,
    NOT,
    NIL,
    OR,
    PASS,
    RAISE,
    RET,
    SUPER,
    TRY,
    TRUE,
    USE,
    WHILE,
    XOR,
    YIELD,
} TokenType;

typedef struct Token
{
    const char *token;
    size_t type;
    size_t indent;
    int length;
    size_t line;
    size_t col;
} Token;

typedef struct keywords
{
    int key;
    char *keyword;
    int len;
} Keywords;

extern Keywords keywords[31];

void printToken(Token *Token);

int iskeyword(const char *s, int len);

Token newToken(const char *token, size_t type, size_t len, size_t col, size_t indent, size_t line);

#endif