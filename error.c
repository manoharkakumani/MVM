#include "error.h"

void errorAt(Compiler *compiler, Token token, const char *message)
{
    if (compiler->parser->panicMode)
        return;
    compiler->parser->panicMode = true;
    fprintf(stderr, "[%ld : %ld] Syntax Error", token.line, token.col);
    if (token.type == END)
    {
        fprintf(stderr, " at end");
    }
    else if (token.type == ERROR)
    {
        // Nothing.
    }
    else
    {
        fprintf(stderr, " at '%.*s'", (int)token.length, token.token);
    }
    fprintf(stderr, ": %s\n", message);
    compiler->parser->hadError = true;
}

void errorAtCurrent(Compiler *compiler, const char *message)
{
    errorAt(compiler, compiler->parser->current, message);
}

void error(Compiler *compiler, const char *message)
{
    errorAt(compiler, compiler->parser->previous, message);
}