#include "expression.h"
#include "statement.h"
#include "bytecode.h"
#include "datatypes/datatypes.h"
#include "error.h"

void integer_(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    emitConstant(compiler, (NEW_INT(compiler->parser->vm, strtol(compiler->parser->previous.token, NULL, 10))));
}

void double_(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    emitConstant(compiler, (NEW_DOUBLE(compiler->parser->vm, strtod(compiler->parser->previous.token, NULL))));
}

void string_(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    emitConstant(compiler, (NEW_STRING(compiler->parser->vm, compiler->parser->previous.token, compiler->parser->previous.length)));
}
void literal(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    switch (compiler->parser->previous.type)
    {
    case FALSE:
        emitByte(compiler, OP_FALSE);
        break;
    case NIL:
        emitByte(compiler, OP_NIL);
        break;
    case TRUE:
        emitByte(compiler, OP_TRUE);
        break;
    default:
        return; // Unreachable.
    }
}

void unary(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    TokenType operatorType = compiler->parser->previous.type;
    parsePrecedence(compiler, PREC_UNARY);
    switch (operatorType)
    {
    case EXCMARK:
        emitByte(compiler, OP_NOT);
        break;
    case MINUS:
        emitByte(compiler, OP_NEG);
        break;
    case PLUS:
        emitByte(compiler, OP_POS);
        break;
    default:
        return;
    }
}

void binary(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    TokenType operatorType = compiler->parser->previous.type;
    ParseRule *rule = getRule(operatorType);
    parsePrecedence(compiler, (Precedence)(rule->precedence + 1));
    u32 inplace = 0;
    switch (operatorType)
    {
    case NEQUAL:
        emitBytes(compiler, OP_EQUAL, inplace + 1);
        emitByte(compiler, OP_NOT);
        break;
    case DEQUAL:
        emitBytes(compiler, OP_EQUAL, inplace);
        break;
    case GREATER:
        emitBytes(compiler, OP_GREATER, inplace);
        break;
    case EGREATER:
        emitBytes(compiler, OP_LESS, inplace + 1);
        emitByte(compiler, OP_NOT);
        break;
    case LESS:
        emitBytes(compiler, OP_LESS, inplace);
        break;
    case ELESS:
        emitBytes(compiler, OP_GREATER, inplace + 1);
        emitByte(compiler, OP_NOT);
        break;
    case PLUS:
        emitBytes(compiler, OP_ADD, inplace);
        break;
    case MINUS:
        emitBytes(compiler, OP_SUB, inplace);
        break;
    case STAR:
        emitBytes(compiler, OP_MUL, inplace);
        break;
    case SLASH:
        emitBytes(compiler, OP_DIV, inplace);
        break;
    case DSTAR:
        emitBytes(compiler, OP_POW, inplace);
        break;
    case DSLASH:
        emitBytes(compiler, OP_IDIV, inplace);
        break;
    case PERCENT:
        emitBytes(compiler, OP_MOD, inplace);
        break;
    case DLESS:
        emitBytes(compiler, OP_LSFT, inplace);
        break;
    case DGREATER:
        emitBytes(compiler, OP_RSFT, inplace);
        break;
    case AMPER:
        emitBytes(compiler, OP_BAND, inplace);
        break;
    case VBAR:
        emitBytes(compiler, OP_BOR, inplace);
        break;
    case CAP:
        emitBytes(compiler, OP_BXOR, inplace);
        break;
    default:
        return; // Unreachable.
    }
}

bool assign(Compiler *compiler, bool canAssign, u8 set, u8 get, u8 name)
{
    if (!canAssign)
        return false;
    size_t type = compiler->parser->current.type;
    if (compiler->flags.dontSetVar && type >= EQUAL && type <= EDGREATER)
    {
        errorAtCurrent(compiler, "Assignment is not possible.");
        return false;
    }
    if (matchToken(compiler, EQUAL))
    {
        expression(compiler);
        emitBytes(compiler, set, name);
        return true;
    }
    u32 inplace = 1;
    if (matchToken(compiler, EPLUS))
    {
        emitBytes(compiler, get, name);
        expression(compiler);
        emitBytes(compiler, OP_ADD, inplace);
        emitBytes(compiler, set, name);
        return true;
    }
    else if (matchToken(compiler, EMINUS))
    {
        emitBytes(compiler, get, name);
        expression(compiler);
        emitBytes(compiler, OP_SUB, inplace);
        emitBytes(compiler, set, name);
        return true;
    }
    else if (matchToken(compiler, ESTAR))
    {
        emitBytes(compiler, get, name);
        expression(compiler);
        emitBytes(compiler, OP_MUL, inplace);
        emitBytes(compiler, set, name);
        return true;
    }
    else if (matchToken(compiler, ESLASH))
    {
        emitBytes(compiler, get, name);
        expression(compiler);
        emitBytes(compiler, OP_DIV, inplace);
        emitBytes(compiler, set, name);
        return true;
    }
    else if (matchToken(compiler, EDSTAR))
    {
        emitBytes(compiler, get, name);
        expression(compiler);
        emitBytes(compiler, OP_POW, inplace);
        emitBytes(compiler, set, name);
        return true;
    }
    else if (matchToken(compiler, EDSLASH))
    {
        emitBytes(compiler, get, name);
        expression(compiler);
        emitBytes(compiler, OP_IDIV, inplace);
        emitBytes(compiler, set, name);
        return true;
    }
    else if (matchToken(compiler, EPERCENT))
    {
        emitBytes(compiler, get, name);
        expression(compiler);
        emitBytes(compiler, OP_MOD, inplace);
        emitBytes(compiler, set, name);
        return true;
    }
    else if (matchToken(compiler, EVBAR))
    {
        emitBytes(compiler, get, name);
        expression(compiler);
        emitBytes(compiler, OP_BOR, inplace);
        emitBytes(compiler, set, name);
        return true;
    }
    else if (matchToken(compiler, EAMPER))
    {
        emitBytes(compiler, get, name);
        expression(compiler);
        emitBytes(compiler, OP_BAND, inplace);
        emitBytes(compiler, set, name);
        return true;
    }
    else if (matchToken(compiler, ECAP))
    {
        emitBytes(compiler, get, name);
        expression(compiler);
        emitBytes(compiler, OP_BXOR, inplace);
        emitBytes(compiler, set, name);
        return true;
    }
    else if (matchToken(compiler, EDGREATER))
    {
        emitBytes(compiler, get, name);
        expression(compiler);
        emitBytes(compiler, OP_RSFT, inplace);
        emitBytes(compiler, set, name);
        return true;
    }
    else if (matchToken(compiler, EDLESS))
    {
        emitBytes(compiler, get, name);
        expression(compiler);
        emitBytes(compiler, OP_LSFT, inplace);
        emitBytes(compiler, set, name);
        return true;
    }
    return false;
}

void dot(Compiler *compiler, bool canAssign)
{
    consumeToken(compiler, NAME, "Expect property name after '.'.");
    u32 name = identifierConstant(compiler, &compiler->parser->previous);
    if (!assign(compiler, canAssign, OP_SETP, OP_AGETP, name))
    {
        emitBytes(compiler, OP_GETP, name);
    }
}

void variable(Compiler *compiler, bool canAssign)
{
    u8 name = identifierConstant(compiler, &compiler->parser->previous);
    u8 set, get;
    set = OP_SETV;
    get = OP_GETV;
    if (assign(compiler, canAssign, set, get, name))
    {
        // do nothing
    }
    else
    {
        if (compiler->parser->previous.type == NAME && checkToken(compiler, ARROW))
        {
            Compiler *arrowCompiler = initCompiler(compiler->parser->vm, compiler->parser, FN_ARROWFN);
            arrowCompiler->function->name = newString(compiler->parser->vm, "arrowfn", 7);
            arrowCompiler->function->argc = 1;
            MyMoString *arg = newString(compiler->parser->vm, compiler->parser->previous.token, compiler->parser->previous.length);
            advanceToken(compiler);
            arrowCompiler->function->argv[0] = arg;
            size_t indent = getIndent(compiler);
            if (matchToken(arrowCompiler, NEWLINE))
            {
                block(arrowCompiler, indent);
                retreatNewLine(arrowCompiler);
            }
            else
            {
                expression(arrowCompiler);
                emitByte(arrowCompiler, OP_FRET);
            }
            MyMoFunction *arrowFunction = endFunction(arrowCompiler);
            emitBytes(compiler, OP_FN, makeConstant(compiler, AS_OBJECT(arrowFunction)));
            // printf("arrow function");
        }
        else
            emitBytes(compiler, get, name);
    }
}

void expression(Compiler *compiler)
{
    parsePrecedence(compiler, PREC_ASSIGNMENT);
    if (checkToken(compiler, COMMA) && !compiler->flags.argv && !compiler->flags.dict && !compiler->flags.list && !compiler->flags.tuple)
    {
        advanceToken(compiler);
        u32 count = 1;
        do
        {
            if (checkToken(compiler, NEWLINE))
            {
                break;
            }
            compiler->flags.dontSetVar++;
            parsePrecedence(compiler, PREC_ASSIGNMENT);
            compiler->flags.dontSetVar--;
            count++;
        } while (matchToken(compiler, COMMA));
        if (count > 1)
        {
            emitBytes(compiler, OP_TUPLE, count);
        }
    }
}

void grouping(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    if (checkArrow(compiler))
    {
        Compiler *arrowCompiler = initCompiler(compiler->parser->vm, compiler->parser, FN_ARROWFN);
        arrowCompiler->function->name = newString(compiler->parser->vm, "arrowfn", 7);
        if (!checkToken(compiler, RPAR))
        {
            do
            {
                advanceToken(arrowCompiler);
                if (arrowCompiler->function->argc == 255)
                {
                    errorAtCurrent(arrowCompiler, "Can't have more than 255 parameters.");
                }
                if (arrowCompiler->parser->previous.type == NAME)
                {
                    arrowCompiler->function->argc++;
                    MyMoString *arg = newString(arrowCompiler->parser->vm, arrowCompiler->parser->previous.token, arrowCompiler->parser->previous.length);
                    arrowCompiler->function->argv[arrowCompiler->function->argc - 1] = arg;
                }
                else
                {
                    error(arrowCompiler, "Expect argument name.");
                }
            } while (matchToken(arrowCompiler, COMMA));
        }
        consumeToken(arrowCompiler, RPAR, "Expect ')' after parameters.");
        size_t indent = getIndent(compiler);
        consumeToken(arrowCompiler, ARROW, "Expect ':' after parameters.");
        if (matchToken(arrowCompiler, NEWLINE))
        {
            block(arrowCompiler, indent);
            retreatNewLine(arrowCompiler);
        }
        else
        {
            expression(arrowCompiler);
            emitByte(arrowCompiler, OP_FRET);
        }
        MyMoFunction *arrowFunction = endFunction(arrowCompiler);
        emitBytes(compiler, OP_FN, makeConstant(compiler, AS_OBJECT(arrowFunction)));
    }
    else
    {
        u32 count = 0;
        compiler->flags.tuple++;
        if (!checkToken(compiler, RPAR))
        {
            do
            {
                skipNewLines(compiler);
                if (checkToken(compiler, RPAR))
                {
                    break;
                }
                compiler->flags.dontSetVar++;
                expression(compiler);
                compiler->flags.dontSetVar--;
                skipNewLines(compiler);
                count++;
            } while (matchToken(compiler, COMMA));
        }
        if (count == 1)
        {
            consumeToken(compiler, RPAR, "Expect ')' after expression.");
        }
        else
        {
            consumeToken(compiler, RPAR, "Expected closing ')'");
            emitBytes(compiler, OP_TUPLE, count);
        }
        compiler->flags.tuple--;
    }
}

u8 argumentList(Compiler *compiler)
{
    uint8_t argCount = 0;
    if (!checkToken(compiler, RPAR))
    {
        do
        {
            if (argCount == 255)
                error(compiler, "Can't have more than 255 arguments.");
            expression(compiler);
            argCount++;
        } while (matchToken(compiler, COMMA));
    }
    consumeToken(compiler, RPAR, "Expect ')' after arguments.");
    return argCount;
}

void call(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    compiler->flags.argv++;
    u8 argCount = argumentList(compiler);
    if (compiler->flags.pithru)
    {
        argCount++;
    }
    emitBytes(compiler, OP_CALL, argCount);
    compiler->flags.argv--;
}

void subScript(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    compiler->flags.dontSetVar++;
    if (matchToken(compiler, COLON))
    {
        emitByte(compiler, OP_NIL);
        if (matchToken(compiler, RSQB))
        {
            emitByte(compiler, OP_NIL);
            emitByte(compiler, OP_NIL);
            emitByte(compiler, OP_SLICE);
            goto endSS;
        }
    step2:
        if (matchToken(compiler, COLON))
        {
            emitByte(compiler, OP_NIL);
        step3:
            if (matchToken(compiler, RSQB))
            {
                emitByte(compiler, OP_NIL);
                emitByte(compiler, OP_SLICE);
                goto endSS;
            }
            else
            {
                expression(compiler);
                consumeToken(compiler, RSQB, "Expect ']' after expression.");
                emitByte(compiler, OP_SLICE);
                goto endSS;
            }
        }
        else
        {
            expression(compiler);
            if (matchToken(compiler, COLON))
                goto step3;
        }
        if (matchToken(compiler, RSQB))
        {
            emitByte(compiler, OP_NIL);
            emitByte(compiler, OP_SLICE);
            goto endSS;
        }
        consumeToken(compiler, RSQB, "Expect ']' after expression.");
    }
    else
    {
        expression(compiler);
        if (matchToken(compiler, RSQB))
        {
            if (matchToken(compiler, EQUAL))
            {
                expression(compiler);
                emitByte(compiler, OP_SETSUBSCR);
            }
            else
            {
                emitByte(compiler, OP_SUBSCR);
            }
            goto endSS;
        }
        if (matchToken(compiler, COLON))
        {
            if (matchToken(compiler, RSQB))
            {
                emitByte(compiler, OP_NIL);
                emitByte(compiler, OP_NIL);
                emitByte(compiler, OP_SLICE);
                goto endSS;
            }
            if (matchToken(compiler, RSQB))
            {
                emitByte(compiler, OP_NIL);
                emitByte(compiler, OP_NIL);
                emitByte(compiler, OP_SLICE);
                goto endSS;
            }
            goto step2;
        }
        consumeToken(compiler, RSQB, "Expect ']' after expression.");
    }
endSS:
    compiler->flags.dontSetVar--;
    return;
}

void list(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    compiler->flags.dontSetVar++;
    u32 count = 0;
    compiler->flags.list++;
    if (!checkToken(compiler, RSQB))
    {
        do
        {
            skipNewLines(compiler);
            if (checkToken(compiler, RSQB))
            {
                break;
            }
            expression(compiler);
            skipNewLines(compiler);
            count++;
        } while (matchToken(compiler, COMMA));
    }
    consumeToken(compiler, RSQB, "Expected closing ']'");
    // if (matchToken(compiler, EQUAL)) {
    //   expression(compiler);
    //   emitBytes(compiler, OP_UNPACK, count);
    // } else {
    emitBytes(compiler, OP_LIST, count);
    // }
    compiler->flags.list--;
    compiler->flags.dontSetVar--;
}

void dictionary(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    compiler->flags.dontSetVar++;
    u32 count = 0;
    if (!checkToken(compiler, RBRACE))
    {
        do
        {
            skipNewLines(compiler);
            if (checkToken(compiler, RBRACE))
            {
                break;
            }
            expression(compiler);
            skipNewLines(compiler);
            consumeToken(compiler, COLON, "Expected ':'");
            skipNewLines(compiler);
            compiler->flags.dict++;
            expression(compiler);
            compiler->flags.dict--;
            skipNewLines(compiler);
            count++;
        } while (matchToken(compiler, COMMA));
    }
    consumeToken(compiler, RBRACE, "Expected closing '}'");
    emitBytes(compiler, OP_DICT, count);
    compiler->flags.dontSetVar--;
}

void pipeThrough(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    compiler->flags.pithru++;
    if (matchToken(compiler, NAME))
    {
        u32 name = identifierConstant(compiler, &compiler->parser->previous);
        emitBytes(compiler, OP_GETV, name);
        if (matchToken(compiler, DOT))
        {
            consumeToken(compiler, NAME, "Expected property name after '.'.");
            u32 name = identifierConstant(compiler, &compiler->parser->previous);
            emitBytes(compiler, OP_GETP, name);
        }
        emitByte(compiler, OP_PITHRU);
        if (matchToken(compiler, LPAR))
        {
            call(compiler, canAssign);
        }
        else
        {
            emitByte(compiler, OP_CALL);
            emitByte(compiler, 1);
        }
    }
    else
    {
        errorAtCurrent(compiler, "Cannot pipe through a literal");
    }
    compiler->flags.pithru--;
}

void yeild(Compiler *compiler, bool canAssign)
{
    UNUSED(canAssign);
    if (compiler->flags.compileType == COMPILE_FUNCTION)
    {
        emitBytes(compiler, OP_GETV, identifierConstant(compiler, &compiler->parser->previous));
        compiler->function->type = (compiler->function->type == FN_METHOD ? FN_GEN_METHOD : FN_GENERATOR);
    }
    else
    {
        errorAt(compiler, compiler->parser->previous, "Cannot yeild outside of a function");
    }
}