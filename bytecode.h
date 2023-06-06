#ifndef __BYTECODE_H__
#define __BYTECODE_H__

#include "compiler.h"

uint makeConstant(Compiler *compiler, MyMoObject *value);
uint identifierConstant(Compiler *compiler, Token *name);

void emitConstant(Compiler *compiler, MyMoObject *value);
void emitByte(Compiler *compiler, u8 byte);
void emitBytes(Compiler *compiler, u8 byte1, u8 byte2);
void patchJump(Compiler *compiler, int offset);
int emitJump(Compiler *compiler, u8 instruction);
void emitLoop(Compiler *compiler, int loopStart);
void emitReturn(Compiler *compiler);

#endif