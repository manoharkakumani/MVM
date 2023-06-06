#ifndef __ERROR_H__
#define __ERROR_H__

#include "common.h"
#include "compiler.h"

void errorAt(Compiler *compiler, Token token, const char *message);
void errorAtCurrent(Compiler *compiler, const char *message);
void error(Compiler *compiler, const char *message);

#endif
