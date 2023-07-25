#include "statement.h"
#include "bytecode.h"
#include "error.h"
#include "expression.h"
#include "memory.h"
#include "datatypes/bool.h"

void synchronize(Compiler *compiler)
{
    compiler->parser->panicMode = false;
    while (compiler->parser->current.type != END)
    {
        if (compiler->parser->previous.type == NEWLINE)
            return;
        switch (compiler->parser->current.type)
        {
        case CLASS:
        case FN:
        case FOR:
        case IF:
        case CASE:
        case WHILE:
        case RET:
            return;
        default:
            // Do nothing.
            ;
        }
        advanceToken(compiler);
    }
}

void and_(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    int andJump = emitJump(compiler, OP_JIF);
    emitByte(compiler, OP_POP);
    parsePrecedence(compiler, PREC_AND);
    patchJump(compiler, andJump);
}

void or_(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    int orJump = emitJump(compiler, OP_JIF);
    int endJump = emitJump(compiler, OP_JMP);
    patchJump(compiler, orJump);
    emitByte(compiler, OP_POP);
    parsePrecedence(compiler, PREC_OR);
    patchJump(compiler, endJump);
}

void trenaryCond(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    int ifJump = emitJump(compiler, OP_JIF);
    emitByte(compiler, OP_POP);
    expression(compiler);
    int jump = emitJump(compiler, OP_JMP);
    patchJump(compiler, ifJump);
    emitByte(compiler, OP_POP);
    consumeToken(compiler, COLON, "expected ':' after expression.");
    expression(compiler);
    patchJump(compiler, jump);
}

void trenaryCond2(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    expression(compiler);
    int ifJump = emitJump(compiler, OP_JIF);
    emitByte(compiler, OP_POP);
    int Jump = emitJump(compiler, OP_JMP);
    patchJump(compiler, ifJump);
    emitByte(compiler, OP_POP);
    consumeToken(compiler, ELSE, "expected 'else' after 'if' expression.");
    emitByte(compiler, OP_POP);
    expression(compiler);
    patchJump(compiler, Jump);
}

void expressionStatement(Compiler *compiler)
{
    expression(compiler);
    emitByte(compiler, OP_POP);
}

void breakStatement(Compiler *compiler)
{
    Loop *loop = compiler->loop;
    if (loop == NULL)
    {
        error(compiler, "cannot use 'break' outside of a loop.");
        return;
    }
    if (loop->breaksCapacity < loop->breaksCount + 1)
    {
        int oldCapacity = loop->breaksCapacity;
        loop->breaksCapacity = ResizeCapacity(oldCapacity);
        loop->breakJumps = ResizeArray(compiler->parser->vm, int, loop->breakJumps, oldCapacity, loop->breaksCapacity);
    }
    compiler->loop->breakJumps[compiler->loop->breaksCount] = emitJump(compiler, OP_JMP);
    compiler->loop->breaksCount++;
    return;
}

void continueStatement(Compiler *compiler)
{
    if (compiler->loop == NULL)
    {
        error(compiler, "cannot use 'continue' outside of a loop.");
        return;
    }
    emitLoop(compiler, compiler->loop->loopStart);
    return;
}

void returnStatement(Compiler *compiler)
{
    if (compiler->function->type == FN_SCRIPT || compiler->function->type == FN_COMPILED)
    {
        error(compiler, "cannot return from top-level code.");
        return;
    }
    if (compiler->function->type == FN_INIT)
    {
        error(compiler, "cannot return from an initializer.");
    }
    if (checkToken(compiler, NEWLINE))
    {
        emitReturn(compiler);
    }
    else
    {
        expression(compiler);
        emitByte(compiler, OP_FRET);
    }
}

void deleteStatement(Compiler *compiler)
{
    if (!matchToken(compiler, NAME))
    {
        errorAtCurrent(compiler, "expected variable");
    }
    u8 name = identifierConstant(compiler, &compiler->parser->previous);
    if (matchToken(compiler, DOT))
    {
        emitBytes(compiler, OP_GETP, name);
        consumeToken(compiler, NAME, "expected property name after '.'.");
        name = identifierConstant(compiler, &compiler->parser->previous);
        emitBytes(compiler, OP_DELP, name);
    }
    else
        emitBytes(compiler, OP_DELV, name);
}

void block(Compiler *compiler, size_t indent)
{
    skipNewLines(compiler);
    if (getIndent(compiler) != (indent + 4))
        errorAtCurrent(compiler, "expected an indented block");
    while (getIndent(compiler) == indent + 4)
    {
        statement(compiler);
        if (checkToken(compiler, END))
            return;
        if (compiler->parser->panicMode)
            synchronize(compiler);
    }
}

void ifStatement(Compiler *compiler)
{
    size_t indent = getIndent(compiler);
    if (matchToken(compiler, IF) || matchToken(compiler, ELIF))
    {
        expression(compiler);
        int ifJump = emitJump(compiler, OP_JIF);
        consumeToken(compiler, COLON, "expected ':' after expression.");
        emitByte(compiler, OP_POP);
        if (matchToken(compiler, NEWLINE))
        {
            block(compiler, indent);
        }
        else
        {
            simpleStatement(compiler);
        }
        int Jump = emitJump(compiler, OP_JMP);
        patchJump(compiler, ifJump);
        emitByte(compiler, OP_POP);
        if (checkToken(compiler, ELIF) && (indent == getIndent(compiler)))
            ifStatement(compiler);
        else if (checkToken(compiler, ELSE) && (indent == getIndent(compiler)))
            elseStatement(compiler);
        patchJump(compiler, Jump);
    }
}

void elseStatement(Compiler *compiler)
{
    size_t indent = getIndent(compiler);
    advanceToken(compiler);
    consumeToken(compiler, COLON, "expected ':' after expression.");
    if (matchToken(compiler, NEWLINE))
    {
        block(compiler, indent);
    }
    else
    {
        simpleStatement(compiler);
    }
}

void startLoop(Compiler *compiler, Loop *loop)
{
    loop->loopStart = currentChunk(compiler)->count;
    loop->enclosing = compiler->loop;
    loop->loopJump = 0;
    loop->breakJumps = NULL;
    loop->breaksCount = 0;
    loop->breaksCapacity = 0;
    compiler->loop = loop;
}

void loopStatement(Compiler *compiler)
{
    Loop loop;
    size_t indent = getIndent(compiler);
    startLoop(compiler, &loop);
    if (matchToken(compiler, WHILE) || matchToken(compiler, FOR))
        expression(compiler);
    consumeToken(compiler, COLON, "expected ':' after expression.");
    compiler->loop->loopJump = emitJump(compiler, OP_JIF);
    emitByte(compiler, OP_POP);
    if (matchToken(compiler, NEWLINE))
    {
        block(compiler, indent);
    }
    else
    {
        simpleStatement(compiler);
    }
    emitLoop(compiler, compiler->loop->loopStart);
    int Jump = emitJump(compiler, OP_JMP);
    compiler->loop = loop.enclosing;
    patchJump(compiler, loop.loopJump);
    emitByte(compiler, OP_POP);
    int breaksCount = loop.breaksCount;
    if (checkToken(compiler, ELSE) && (indent == getIndent(compiler)))
    {
        elseStatement(compiler);
    }
    patchJump(compiler, Jump);
    while (breaksCount)
    {
        breaksCount--;
        patchJump(compiler, loop.breakJumps[breaksCount]);
    }
    if (loop.breaksCapacity)
    {
        FreeArray(compiler->parser->vm, int, loop.breakJumps, loop.breaksCapacity);
    }
}

void caseStatement(Compiler *compiler)
{
    int caseEnds[256];
    int caseCount = 0;
    size_t indent = getIndent(compiler);
    if (matchToken(compiler, CASE))
    {
        expression(compiler);
    }
    consumeToken(compiler, COLON, "expected ':' after expression.");
    if (matchToken(compiler, NEWLINE))
    {
        skipNewLines(compiler);
        if (getIndent(compiler) != (indent + 4))
            errorAtCurrent(compiler, "expected block");
        while (getIndent(compiler) == indent + 4)
        {
            size_t c_indent = getIndent(compiler);
            expression(compiler);
            consumeToken(compiler, COLON, "expected ':' after expression.");
            int CJump = emitJump(compiler, OP_CJMP);
            if (matchToken(compiler, NEWLINE))
            {
                block(compiler, c_indent);
            }
            else
            {
                simpleStatement(compiler);
            }
            caseEnds[caseCount++] = emitJump(compiler, OP_JMP);
            patchJump(compiler, CJump);
            if (checkToken(compiler, DOLLAR))
            {
                elseStatement(compiler);
                // caseEnds[caseCount++] = emitJump(compiler, OP_JMP);
                if (getIndent(compiler) == indent + 4)
                    errorAtCurrent(compiler, "$ must be at the last case");
                else
                    break;
            }
        }
    }
    else
    {
        errorAtCurrent(compiler, "expected an indented block");
    }
    for (int i = 0; i < caseCount; i++)
    {
        patchJump(compiler, caseEnds[i]);
    }
}

void simpleStatement(Compiler *compiler)
{
    if (matchToken(compiler, RET))
    {
        returnStatement(compiler);
    }
    else if (matchToken(compiler, DEL))
    {
        deleteStatement(compiler);
    }
    else if (matchToken(compiler, PASS))
    {
        emitByte(compiler, OP_NOP);
    }
    else if (matchToken(compiler, BREAK))
    {
        breakStatement(compiler);
    }
    else if (matchToken(compiler, CONTINUE))
    {
        continueStatement(compiler);
    }
    else
    {
        expressionStatement(compiler);
    }
    if (compiler->parser->current.type != END)
    {
        consumeToken(compiler, NEWLINE, "expect 'newline' after expression.");
    }
}

MyMoFunction *endFunction(Compiler *compiler)
{
    emitReturn(compiler);
    MyMoFunction *function = compiler->function;
#ifdef DEBUG_PRINT_CODE
    if (!compiler->parser->hadError)
    {
        disassembleChunk(currentChunk(compiler), function->name->value);
    }
#endif
    free(compiler);
    return function;
}

bool getParameter(MyMoString **parameters, MyMoString *name, int count)
{
    for (int c = 0; c < count; c++)
        if (!strcmp(parameters[c]->value, name->value))
            return true;
    return false;
}

void fnParameters(Compiler *compiler)
{
    if (!matchToken(compiler, NAME))
        errorAtCurrent(compiler, "expect parameters.");
    MyMoString *name = newString(compiler->parser->vm, compiler->parser->previous.token, compiler->parser->previous.length);
    if (!getParameter(compiler->function->argv, name, compiler->function->argc))
        compiler->function->argv[compiler->function->argc] = name;
    else
        errorAt(compiler, compiler->parser->previous, "duplicated parameters.");
    compiler->function->argc++;
    // if (matchToken(compiler, EQUAL)){
    //   expression(compiler);
    // }
}

bool operatorMethod(Compiler *compiler, u32 *argc)
{
#define __OP_OVERLOAD__(_argc) \
    do                         \
    {                          \
        *argc = _argc;         \
        return true;           \
    } while (0)
    if (matchToken(compiler, UPLUS))
        __OP_OVERLOAD__(1);
    if (matchToken(compiler, UMINUS))
        __OP_OVERLOAD__(1);
    if (matchToken(compiler, PLUS))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, MINUS))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, EPLUS))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, EMINUS))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, STAR))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, ESTAR))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, SLASH))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, DSTAR))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, ESLASH))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, PERCENT))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, EPERCENT))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, EDSTAR))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, AMPER))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, EAMPER))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, VBAR))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, EVBAR))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, CAP))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, ECAP))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, DLESS))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, EDLESS))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, DGREATER))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, EDGREATER))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, DEQUAL))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, ELESS))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, LESS))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, EGREATER))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, GREATER))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, EXCMARK))
        __OP_OVERLOAD__(1);
    if (matchToken(compiler, NEQUAL))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, IN))
        __OP_OVERLOAD__(2);
    if (matchToken(compiler, NOT))
        __OP_OVERLOAD__(1);
    if (matchToken(compiler, IS))
        __OP_OVERLOAD__(2);
#undef __OP_OVERLOAD__
    return false;
}

void functionStatement(Compiler *compiler)
{
    size_t indent = getIndent(compiler);
    advanceToken(compiler);
    FunctionType type;
    if (compiler->flags.cl_fn)
    {
        if (memcmp(compiler->parser->current.token, "__init__", compiler->parser->current.length) == 0)
        {
            type = FN_INIT;
        }
        else
        {
            type = FN_METHOD;
        }
    }
    else
    {
        type = FN_FUNCTION;
    }
    u32 argc = 0;
    bool oper_meth = operatorMethod(compiler, &argc);
    if (oper_meth)
    {
        type = FN_OPERATOR;
    }
    else if (!matchToken(compiler, NAME))
    {
        errorAtCurrent(compiler, "expected function name");
    }
    u32 name = identifierConstant(compiler, &compiler->parser->previous);
    Compiler *fncompiler = initCompiler(compiler->parser->vm, compiler->parser, type);
    compiler->flags.cl_fn = false;
    if (checkToken(compiler, LPAR) || oper_meth)
    {
        consumeToken(compiler, LPAR, "expected '(' after operator method name");
        if (!checkToken(fncompiler, RPAR) || oper_meth)
        {
            if (oper_meth)
            {
                fnParameters(fncompiler);
                argc--;
                if (argc)
                {
                    if (!matchToken(fncompiler, COMMA))
                        errorAtCurrent(fncompiler, "expected  argument as operator method requires upto two arguments.");
                    fnParameters(fncompiler);
                }
            }
            else
            {

                do
                {
                    if (fncompiler->function->argc > 255)
                    {
                        errorAtCurrent(fncompiler, "cannot have more than 255 parameters.");
                    }
                    fnParameters(fncompiler);
                } while (matchToken(fncompiler, COMMA));
            }
        }
        consumeToken(fncompiler, RPAR, "expected ')' after parameters.");
    }
    consumeToken(fncompiler, COLON, "expected ':' before function body.");
    if (matchToken(fncompiler, NEWLINE))
    {
        block(fncompiler, indent);
    }
    else
    {
        simpleStatement(fncompiler);
    }
    MyMoFunction *function = endFunction(fncompiler);
    switch (type)
    {
    case FN_INIT:
    case FN_METHOD:
    case FN_OPERATOR:
        emitBytes(compiler, OP_MET, makeConstant(compiler, AS_OBJECT(function)));
        emitByte(compiler, name);
        compiler->flags.cl_fn = true;
        break;
    case FN_FUNCTION:
        emitBytes(compiler, OP_FN, makeConstant(compiler, AS_OBJECT(function)));
        emitBytes(compiler, OP_SETV, name);
        emitByte(compiler, OP_POP);
        break;
    default:
        break;
    }
}

void classStatement(Compiler *compiler)
{
    size_t indent = getIndent(compiler);
    advanceToken(compiler);
    if (!matchToken(compiler, NAME))
    {
        errorAtCurrent(compiler, "expected class name");
    }
    u8 name = identifierConstant(compiler, &compiler->parser->previous);
    u8 superClasses = 0;
    emitBytes(compiler, OP_CLASS, name);
    if(matchToken(compiler,LPAR))
    {
    if (!checkToken(compiler, RPAR))
    {
        compiler->flags.tuple = true;
        do
        {
            if (superClasses == 255)
            {
                error(compiler, "cannot have more than 255 super classes.");
            }
            expression(compiler);
            superClasses++;
        } while (matchToken(compiler, COMMA));
        compiler->flags.tuple = false;
    }
    if(superClasses){
      emitBytes(compiler,OP_SUPERARGS,makeConstant(compiler,NEW_INT(compiler->parser->vm,superClasses)));
    }
    consumeToken(compiler, RPAR, "expected ')' after arguments.");
    }
    consumeToken(compiler, COLON, "expected ':' before class body.");
    if (matchToken(compiler, NEWLINE))
    {
        compiler->flags.cl_fn = true;
        block(compiler, indent);
    }
    else
    {
        simpleStatement(compiler);
    }
    emitByte(compiler, OP_ENDCLASS);
    emitBytes(compiler, OP_SETV, name);
    emitByte(compiler, OP_POP);
    compiler->flags.cl_fn = false;
}

void compoundStatement(Compiler *compiler)
{
    switch (compiler->parser->current.type)
    {
    case CLASS:
        classStatement(compiler);
        break;
    case FN:
        functionStatement(compiler);
        break;
    default:
        errorAtCurrent(compiler, "Invalid compound statement");
        break;
    }
}
void flowStatement(Compiler *compiler)
{
    switch (compiler->parser->current.type)
    {
    case IF:
        ifStatement(compiler);
        break;
    case ELIF:
        errorAtCurrent(compiler, "Unexpected elif with out if statement");
        matchToken(compiler, ELIF);
        break;
    case ELSE:
        errorAtCurrent(compiler, "Unexpected else with out if or while statement");
        matchToken(compiler, ELSE);
        break;
    case CASE:
        caseStatement(compiler);
        break;
    case FOR:
    case WHILE:
        loopStatement(compiler);
        break;
    default:
        errorAtCurrent(compiler, "Invalid flow statement");
    }
}

void fromStatement(Compiler *compiler)
{
    advanceToken(compiler);
    if (!matchToken(compiler, STRING))
    { 
        errorAtCurrent(compiler, "expected module name Ex:- 'from \"module\" use abc'");
    }
    u32 path = identifierConstant(compiler, &compiler->parser->previous);
    consumeToken(compiler, USE, "expected 'use' after module name");
    if(matchToken(compiler, STAR)){
        emitByte(compiler, OP_FALSE);
        emitBytes(compiler, OP_USE, path);
        consumeToken(compiler,NEWLINE, "expected newline after * iin use statement");
        emitByte(compiler,OP_COPY);
        return;        
    }
multiFrom:
    emitByte(compiler, OP_FALSE);
    emitBytes(compiler, OP_USE, path);
    if (!matchToken(compiler, NAME))
    {
        errorAtCurrent(compiler, "expected property name Ex:- 'from \"module\" use abc'");
    }
    u32 name = identifierConstant(compiler, &compiler->parser->previous);
    emitBytes(compiler, OP_GETP, name);
    if (checkToken(compiler, AS))
    {
        advanceToken(compiler);
        if (!matchToken(compiler, NAME))
        {
            errorAtCurrent(compiler, "expected module name  variable Ex:- 'from \"module\" use abc as a'");
        }    
        name = identifierConstant(compiler, &compiler->parser->previous);
    }
    emitBytes(compiler, OP_SETV, name);
    emitByte(compiler, OP_POP);
    if (matchToken(compiler, NEWLINE) || matchToken(compiler, END))
    {
        return;
    }
    else if (matchToken(compiler, COMMA))
    {
        goto multiFrom;
    }
    else
    {
        errorAtCurrent(compiler, "expected newline or ',' after use statement");
    }
}

void useStatement(Compiler *compiler)
{
multiUse:
    advanceToken(compiler);
    int noAs = 1;
    if (!matchToken(compiler, STRING))
    {
        errorAtCurrent(compiler, "expected module name Ex:- 'use \"module\"'");
    }
    u32 path = identifierConstant(compiler, &compiler->parser->previous);
    emitByte(compiler, OP_TRUE);
    emitBytes(compiler, OP_USE, path);
    if (checkToken(compiler, AS))
    {
        advanceToken(compiler);
        if (!matchToken(compiler, NAME))
        {
            errorAtCurrent(compiler, "expected module name  variable Ex:- 'use \"module\" as mod'");
        }
        noAs = 0;
        emitByte(compiler, OP_POP);
        emitBytes(compiler, OP_SETV, identifierConstant(compiler, &compiler->parser->previous));
        emitByte(compiler, OP_POP);
    }
    if (matchToken(compiler, NEWLINE) || matchToken(compiler, END))
    {
        if (noAs)
        {
            emitByte(compiler, OP_SETM);
        }
        return;
    }
    else if (matchToken(compiler, COMMA))
    {
        goto multiUse;
    }
    else
    {
        errorAtCurrent(compiler, "expected newline or ',' after use statement");
    }
}

void moduleStatement(Compiler *compiler)
{
    switch (compiler->parser->current.type)
    {
    case USE:
        useStatement(compiler);
        break;
    case FROM:
        fromStatement(compiler);
        break;
    default:
        errorAtCurrent(compiler, "Invalid package statement");
        break;
    }
}
void statement(Compiler *compiler)
{
    if (checkToken(compiler, IF) || checkToken(compiler, ELIF) || checkToken(compiler, ELSE) || checkToken(compiler, CASE) || checkToken(compiler, WHILE) || checkToken(compiler, FOR))
        flowStatement(compiler);
    else if (checkToken(compiler, CLASS) || checkToken(compiler, FN))
        compoundStatement(compiler);
    else if (checkToken(compiler, USE) || checkToken(compiler, FROM))
    {
        moduleStatement(compiler);
    }
    else
    {
        simpleStatement(compiler);
    }
}

void statements(Compiler *compiler)
{
    if (getIndent(compiler))
    {
        errorAtCurrent(compiler, "Invalid indentation");
    }
    while (!getIndent(compiler))
    {
        statement(compiler);
        if (compiler->parser->panicMode)
            synchronize(compiler);
        if (matchToken(compiler, END))
            break;
        if (getIndent(compiler))
            errorAtCurrent(compiler, "Invalid indentation");
    }
}