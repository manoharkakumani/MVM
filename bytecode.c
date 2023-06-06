
#include "bytecode.h"
#include "error.h"
#include "datatypes/datatypes.h"

uint makeConstant(Compiler *compiler, MyMoObject *value)
{
    int constant = addConstant(compiler->parser->vm, currentChunk(compiler), value);
    if (constant > UINT16_MAX)
    {
        error(compiler, "Too many constants in one chunk.");
        return 0;
    }
    return (uint)constant;
}

uint identifierConstant(Compiler *compiler, Token *name)
{
    return makeConstant(compiler, NEW_STRING(compiler->parser->vm, name->token, name->length));
}

void emitConstant(Compiler *compiler, MyMoObject *value)
{
    emitBytes(compiler, OP_CONST, makeConstant(compiler, value));
}
void emitByte(Compiler *compiler, u8 byte)
{
    writeChunk(compiler->parser->vm, currentChunk(compiler), byte, compiler->parser->previous.line, compiler->parser->previous.col);
}

void emitBytes(Compiler *compiler, u8 byte1, u8 byte2)
{
    emitByte(compiler, byte1);
    emitByte(compiler, byte2);
}

void patchJump(Compiler *compiler, int offset)
{
    int jump = currentChunk(compiler)->count - offset - 2;
    if (jump > UINT16_MAX)
    {
        error(compiler, "Too much code to jump over.");
    }
    currentChunk(compiler)->code[offset] = (jump >> 8) & 0xff;
    currentChunk(compiler)->code[offset + 1] = jump & 0xff;
}

int emitJump(Compiler *compiler, u8 instruction)
{
    emitByte(compiler, instruction);
    emitByte(compiler, 0xff);
    emitByte(compiler, 0xff);
    return currentChunk(compiler)->count - 2;
}

void emitLoop(Compiler *compiler, int loopStart)
{
    emitByte(compiler, OP_LOOP);
    int offset = currentChunk(compiler)->count - loopStart + 2;
    if (offset > UINT16_MAX)
        error(compiler, "Loop body too large.");
    emitByte(compiler, (offset >> 8) & 0xff);
    emitByte(compiler, offset & 0xff);
}

void emitReturn(Compiler *compiler)
{
    switch (compiler->function->type)
    {
    case FN_SCRIPT:
    case FN_MODULE:
        emitByte(compiler, OP_RET);
        break;
        // case INIT:
        //   emitByte(compiler,OP_FRET);
        //   break;
    default:
        emitByte(compiler, OP_NIL);
        emitByte(compiler, OP_FRET);
        break;
    }
}