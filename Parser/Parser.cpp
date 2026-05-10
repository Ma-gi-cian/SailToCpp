#include "Parser.hpp"
#include "AST.hpp"
#include <sstream>

using namespace node;

const Token & Parser::peek(size_t offset) const {
    size_t idx = s_pos + offset;
    if(idx >= s_tokens.size()){
        return s_tokens.back();
    }
    return s_tokens[idx];
}

Token Parser::advance()
{
    Token t = s_tokens[s_pos];
    if(s_pos + 1 < s_tokens.size()) {
        ++s_pos;
    }
    return t;
}

bool Parser::check(TokenType t) const 
{
    // Check if the current token in the s_tokens is of the type we are checking
    return peek().sType == t;
}

bool Parser::match(TokenType t)
{
    if(check(t)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::expect(TokenType t, const std::string & context)
{
    if(!check(t)){
        std::ostringstream oss;
        oss << "[Line " << peek().linenumber << "]" << context << ": expected token type "  << static_cast<int>(t) << " but got '" << peek().sText << "'";
        throw ParseError(oss.str(), peek());
    }
    return advance();
}

bool Parser::atEnd() const 
{
    return peek().sType == TokenType::EOF_TYPE;
}

void Parser::error(const std::string & msg, const Token & t)
{
    std::ostringstream oss;
    oss << "[Line " << t.linenumber << " ] ";
    if(t.filepath) {
        oss << t.filepath << " : ";
    }
    oss << msg << " ( near '" << t.sText << "')";
    throw ParseError(oss.str(), t);;
}


// NOw we are dealing with the different AST notes

std::unique_ptr<Program> Parser::parse()
{
    auto program = std::make_unique<Program>();

    while(!atEnd())
    {
        // Expecting "function clause" here 
        if(peek().sType == TokenType::FUNCTION_CLAUSE)
        {
            program->clauses.push_back(parseFunctionClause());
        } else {
            error("Expected function clause at top level ", peek());
        }
    }
    return program;
};


// Now we will get the name of the extension here

std::unique_ptr<FunctionClause> Parser::parseFunctionClause()
{
    auto clause = std::make_unique<FunctionClause>();
    clause->line = peek().linenumber;

    expect(TokenType::FUNCTION_CLAUSE, "parseFunctionClause");
    expect(TokenType::EXECUTE, "parseFunctionClause: expected 'execute'");

    expect(TokenType::LPAREN, "parseFunctionClause: expected '(' before pattern");

    // Now we will get the name of the extension
    clause->pattern = parseInstructionPattern();

    // Now we will just consume the ')'
    expect(TokenType::RPAREN, "parseFunctionClause: expected ')' before pattern");

    // consume the '='
    expect(TokenType::EQUAL, "parseFunctionClause: expected '=' before pattern");

    // consume the '{'
    expect(TokenType::LBRACE, "parseFunctionClause: expected '{'");

    // Now we are in the body area - so we will go until we get the entire thing uptil the '}'

    while(!check(TokenType::RBRACE) && !atEnd())
    {
        clause->body.push_back(parseStatement());
    }


    // consume the '}'
    expect(TokenType::RBRACE, "parseFunctionClause: expected '}'");

    return clause;
}

std::unique_ptr<InstructionPattern> Parser::parseInstructionPattern()
{
    auto pat = std::make_unique<InstructionPattern>();
    pat->line = peek().linenumber;
 
    // mnemonic — should be an IDENTIFIER (e.g. SHA256SIG0)
    Token mnemTok = expect(TokenType::IDENTIFIER,
                           "parseInstructionPattern: expected mnemonic");
    pat->mnemonic = mnemTok.sText;
 
    // operand list  ( rs1, rd )
    expect(TokenType::LPAREN, "parseInstructionPattern: expected '('");
 
    while (!check(TokenType::RPAREN) && !atEnd())
    {
        Token op = expect(TokenType::IDENTIFIER,
                          "parseInstructionPattern: expected operand name");
        pat->operands.push_back(op.sText);
        if (!match(TokenType::COMMA))
            break;
    }
 
    expect(TokenType::RPAREN, "parseInstructionPattern: expected ')'");
 
    return pat;
}


// Now body parsing will be done here 

NodePtr Parser::parseStatement()
{
    if (check(TokenType::LET))
    {
        return parseLetBinding();
    }

    if((check(TokenType::X_REG) || check(TokenType::F_REG)) && peek(1).sType == TokenType::LPAREN)
    {
        return parseRegAssign();
    }

    if(check(TokenType::RETIRE_FAIL) || check(TokenType::RETIRE_SUCCESS))
    {
        auto ret = std::make_unique<RetireStmt>();
        ret->line = peek().linenumber;
        ret->success = (peek().sType == TokenType::RETIRE_SUCCESS);
        advance();
        return ret;
    }

    auto stmt = std::make_unique<ExprStmt>();
    stmt->line = peek().linenumber;
    stmt->expr = parseExpr();
    match(TokenType::SEMICOLON);
    return stmt;
}

NodePtr Parser::parseLetBinding()
{
    auto let = std::make_unique<LetBinding>();
    let->line = peek().linenumber;
 
    expect(TokenType::LET, "parseLetBinding");
 
    Token nameTok = expect(TokenType::IDENTIFIER,
                           "parseLetBinding: expected variable name");
    let->name = nameTok.sText;
 
    // optional type annotation  :  bits(32)
    if (match(TokenType::COLON))
    {
        // type name — bits / bool / int / nat / unit / vector
        Token typeTok = peek();
        if (typeTok.sType == TokenType::BITS  || typeTok.sType == TokenType::BOOL ||
            typeTok.sType == TokenType::INT   || typeTok.sType == TokenType::NAT  ||
            typeTok.sType == TokenType::UNIT  || typeTok.sType == TokenType::VECTOR ||
            typeTok.sType == TokenType::IDENTIFIER)
        {
            let->typeName = typeTok.sText;
            advance();
 
            // optional  (N)  type parameter
            if (match(TokenType::LPAREN))
            {
                let->typeParam = parseNumLiteralValue();
                expect(TokenType::RPAREN, "parseLetBinding: expected ')' after type param");
            }
        }
        else
        {
            error("Expected type name after ':'", typeTok);
        }
    }
 
    expect(TokenType::EQUAL, "parseLetBinding: expected '='");
 
    let->init = parseExpr();
 
    match(TokenType::SEMICOLON);
    return let;
}

NodePtr Parser::parseIfExpr()
{
    auto ifExpr = std::make_unique<IfExpr>();
    ifExpr->line = peek().linenumber;

    expect(TokenType::IF, "parseIfExpr");

    ifExpr->condition = parseExpr();

    expect(TokenType::THEN, "parseIfExpr: expected 'then'");

    ifExpr->thenBranch = parseExpr();

    expect(TokenType::ELSE, "parseIfExpr: expected 'else'");

    ifExpr->elseBranch = parseExpr();

    return ifExpr;
}


NodePtr Parser::parseRegAssign()
{
    auto assign = std::make_unique<RegAssign>();
    assign->line = peek().linenumber;
 
    Token regFileTok = advance();  // X or F
    assign->regFile  = regFileTok.sText;
 
    expect(TokenType::LPAREN, "parseRegAssign: expected '('");
    Token regNameTok = expect(TokenType::IDENTIFIER,
                              "parseRegAssign: expected register name");
    assign->regName  = regNameTok.sText;
    expect(TokenType::RPAREN, "parseRegAssign: expected ')'");
 
    expect(TokenType::EQUAL, "parseRegAssign: expected '='");
 
    assign->value = parseExpr();
 
    match(TokenType::SEMICOLON);
    return assign;
}
 
// =============================================================================
// expression parsing — precedence climbing (low → high precedence)
// 
// TODO: Check how do others handle this - there should be a structures method of doing precedance handling
//  parseExpr        ->  XOR  (lowest)
//  parseXorExpr     ->  OR
//  parseOrExpr      ->  AND
//  parseAndExpr     ->  shift
//  parseShiftExpr   ->  add/sub
//  parseAddExpr     ->  mul/div
//  parseMulExpr     ->  unary
//  parseUnaryExpr   ->  postfix  (bit-slice)
//  parsePostfixExpr ->  primary  (highest)
// =============================================================================


// Concatenation '@' and XOR '^'  (same level in Sail)
NodePtr Parser::parseExpr()
{
    NodePtr lhs = parseXorExpr();
 
    while (check(TokenType::AT) || check(TokenType::AT_EQUAL))
    {
        std::string op = peek().sText;
        advance();
        auto bin  = std::make_unique<BinaryExpr>();
        bin->line = lhs->line;
        bin->op   = op;
        bin->lhs  = std::move(lhs);
        bin->rhs  = parseXorExpr();
        lhs = std::move(bin);
    }
    return lhs;
}
 
NodePtr Parser::parseXorExpr()
{
    NodePtr lhs = parseOrExpr();
 
    while (check(TokenType::XOR))
    {
        advance();
        auto bin  = std::make_unique<BinaryExpr>();
        bin->line = lhs->line;
        bin->op   = "^";
        bin->lhs  = std::move(lhs);
        bin->rhs  = parseOrExpr();
        lhs = std::move(bin);
    }
    return lhs;
}
 
NodePtr Parser::parseOrExpr()
{
    NodePtr lhs = parseAndExpr();
 
    while (check(TokenType::OR) || check(TokenType::OR_OR))
    {
        std::string op = peek().sText;
        advance();
        auto bin  = std::make_unique<BinaryExpr>();
        bin->line = lhs->line;
        bin->op   = op;
        bin->lhs  = std::move(lhs);
        bin->rhs  = parseAndExpr();
        lhs = std::move(bin);
    }
    return lhs;
}
 
NodePtr Parser::parseAndExpr()
{
    NodePtr lhs = parseComparisonExpr();
 
    while (check(TokenType::AND) || check(TokenType::AND_AND))
    {
        std::string op = peek().sText;
        advance();
        auto bin  = std::make_unique<BinaryExpr>();
        bin->line = lhs->line;
        bin->op   = op;
        bin->lhs  = std::move(lhs);
        bin->rhs  = parseComparisonExpr();
        lhs = std::move(bin);
    }
    return lhs;
}
 
NodePtr Parser::parseShiftExpr()
{
    NodePtr lhs = parseAddExpr();
 
    while (check(TokenType::LSHIFT)      ||
           check(TokenType::RSHIFT)      ||
           check(TokenType::ARITH_RSHIFT))
    {
        std::string op = peek().sText;
        advance();
        auto bin  = std::make_unique<BinaryExpr>();
        bin->line = lhs->line;
        bin->op   = op;
        bin->lhs  = std::move(lhs);
        bin->rhs  = parseAddExpr();
        lhs = std::move(bin);
    }
    return lhs;
}
 
NodePtr Parser::parseAddExpr()
{
    NodePtr lhs = parseMulExpr();
 
    while (check(TokenType::PLUS) || check(TokenType::MINUS))
    {
        std::string op = peek().sText;
        advance();
        auto bin  = std::make_unique<BinaryExpr>();
        bin->line = lhs->line;
        bin->op   = op;
        bin->lhs  = std::move(lhs);
        bin->rhs  = parseMulExpr();
        lhs = std::move(bin);
    }
    return lhs;
}
 
NodePtr Parser::parseMulExpr()
{
    NodePtr lhs = parseUnaryExpr();
 
    while (check(TokenType::STAR)   ||
           check(TokenType::SLASH)  ||
           check(TokenType::MODULO))
    {
        std::string op = peek().sText;
        advance();
        auto bin  = std::make_unique<BinaryExpr>();
        bin->line = lhs->line;
        bin->op   = op;
        bin->lhs  = std::move(lhs);
        bin->rhs  = parseUnaryExpr();
        lhs = std::move(bin);
    }
    return lhs;
}
 
NodePtr Parser::parseUnaryExpr()
{
    if (check(TokenType::NOT)   ||   // ~
        check(TokenType::BANG)  ||   // !
        check(TokenType::MINUS))     // -
    {
        std::string op = peek().sText;
        size_t line    = peek().linenumber;
        advance();
        auto u   = std::make_unique<UnaryExpr>();
        u->line  = line;
        u->op    = op;
        u->operand = parseUnaryExpr();
        return u;
    }
    return parsePostfixExpr();
}
 
// Handles  expr[hi..lo]  bit-slice postfix
NodePtr Parser::parsePostfixExpr()
{
    NodePtr base = parsePrimaryExpr();
 
    if (check(TokenType::LBRACKET))
    {
        // Must have been a RegRead to form a BitSlice
        auto * rr = dynamic_cast<RegRead*>(base.get());
        if (!rr)
            error("Bit-slice '[..]' can only be applied to a register read", peek());
 
        advance(); // consume '['
 
        auto slice   = std::make_unique<BitSlice>();
        slice->line  = base->line;
 
        // hi
        auto hi      = std::make_unique<NumLiteral>();
        hi->value    = parseNumLiteralValue();
        slice->hi    = std::move(hi);
 
        expect(TokenType::DOTDOT, "BitSlice: expected '..'");
 
        // lo
        auto lo      = std::make_unique<NumLiteral>();
        lo->value    = parseNumLiteralValue();
        slice->lo    = std::move(lo);
 
        expect(TokenType::RBRACKET, "BitSlice: expected ']'");
 
        // transfer ownership of the RegRead into the slice
        auto data = base.release();
        slice->source = std::unique_ptr<RegRead>(rr);
 
        return slice;
    }
 
    return base;
}
 
// =============================================================================
// Primary expressions
// =============================================================================
 
NodePtr Parser::parsePrimaryExpr()
{
    const Token & t = peek();
 
    if (t.sType == TokenType::INTEGER_LITERAL ||
        t.sType == TokenType::BIT_LITERAL     ||
        t.sType == TokenType::HEX_LITERAL)
    {
        auto lit   = std::make_unique<NumLiteral>();
        lit->line  = t.linenumber;
        lit->value = parseNumLiteralValue();
        return lit;
    }
 
    if (t.sType == TokenType::X_REG || t.sType == TokenType::F_REG)
    {
        return parseRegReadOrSlice(t.sText);
    }
 
    if (t.sType == TokenType::ROR            ||
        t.sType == TokenType::ROL            ||
        t.sType == TokenType::SAIL_ZEROS     ||
        t.sType == TokenType::SAIL_ONES      ||
        t.sType == TokenType::REPLICATE_BITS ||
        t.sType == TokenType::TRUNCATE       ||
        t.sType == TokenType::ZERO_EXTEND    ||
        t.sType == TokenType::SIGN_EXTEND    ||
        t.sType == TokenType::UNSIGNED       ||
        t.sType == TokenType::SIGNED         ||
        t.sType == TokenType::EXTS           ||
        t.sType == TokenType::EXTZ)
    {
        return parseBuiltinCall(t.sText, t.linenumber);
    }
 
    if (t.sType == TokenType::IDENTIFIER)
    {
        std::string name = t.sText;
        size_t line      = t.linenumber;
        advance();
 
        // If followed by '(' it's a generic function call
        if (check(TokenType::LPAREN))
            return parseBuiltinCall(name, line);   // reuse same builder
 
        // Plain identifier
        auto id   = std::make_unique<Identifier>();
        id->line  = line;
        id->name  = name;
        return id;
    }

    if (t.sType == TokenType::IF)
    {
        auto ifExpr = std::make_unique<IfExpr>();
        ifExpr->line = peek().linenumber;
        
        advance(); // consume 'if'
        
        // Parse condition - this will consume "xlen == 32"
        ifExpr->condition = parseExpr();  // Use a lower precedence level, not parseExpr()
        
        expect(TokenType::THEN, "parseIfExpr: expected 'then'");
        
        // Parse then branch
        ifExpr->thenBranch = parseExpr();
        
        expect(TokenType::ELSE, "parseIfExpr: expected 'else'");
        
        // Parse else branch  
        ifExpr->elseBranch = parseExpr();
        
        return ifExpr;
    } 
 
    if (t.sType == TokenType::LPAREN)
    {
        advance(); // consume '('
        NodePtr inner = parseExpr();
        expect(TokenType::RPAREN, "parsePrimaryExpr: expected closing ')'");
        return inner;
    }
 
    error("Unexpected token in expression", t);
}
 
 
NodePtr Parser::parseRegReadOrSlice(const std::string & regFile)
{
    size_t line = peek().linenumber;
    advance(); // consume 'X' or 'F'
 
    expect(TokenType::LPAREN, "RegRead: expected '('");
    Token regNameTok = expect(TokenType::IDENTIFIER, "RegRead: expected register name");
    expect(TokenType::RPAREN, "RegRead: expected ')'");
 
    auto rr       = std::make_unique<RegRead>();
    rr->line      = line;
    rr->regFile   = regFile;
    rr->regName   = regNameTok.sText;
    return rr;
}
 
// Parses  name(arg, arg, ...)  — caller has already peeked the name token
NodePtr Parser::parseBuiltinCall(const std::string & name, size_t line)
{
    // If we arrived here from parsePrimaryExpr for a builtin, the name token
    // is still in the stream; for IDENTIFIER we already consumed it.
    // Consume only if the current token is still the name.
    if (peek().sText == name &&
        peek().sType != TokenType::LPAREN)
        advance();
 
    auto call  = std::make_unique<BuiltinCall>();
    call->line = line;
    call->name = name;
 
    expect(TokenType::LPAREN, "BuiltinCall: expected '(' after " + name);
 
    while (!check(TokenType::RPAREN) && !atEnd())
    {
        call->args.push_back(parseExpr());
        if (!match(TokenType::COMMA))
            break;
    }
 
    expect(TokenType::RPAREN, "BuiltinCall: expected ')' after args of " + name);
 
    return call;
}

NodePtr Parser::parseComparisonExpr()
{
    NodePtr lhs = parseShiftExpr();
    while (check(TokenType::EQUAL_EQUAL)   ||
           check(TokenType::NOT_EQUAL)     ||
           check(TokenType::LESS)          ||
           check(TokenType::LESS_EQUAL)    ||
           check(TokenType::GREATER)       ||
           check(TokenType::GREATER_EQUAL))
    {
        std::string op = peek().sText;
        advance();
        auto bin  = std::make_unique<BinaryExpr>();
        bin->line = lhs->line;
        bin->op   = op;
        bin->lhs  = std::move(lhs);
        bin->rhs  = parseShiftExpr();
        lhs = std::move(bin);
    }
    return lhs;
}
 
// Consume and convert an integer/bit/hex literal token to uint64_t
uint64_t Parser::parseNumLiteralValue()
{
    const Token & t = peek();
 
    if (t.sType == TokenType::INTEGER_LITERAL)
    {
        uint64_t v = std::stoull(t.sText);
        advance();
        return v;
    }
    if (t.sType == TokenType::HEX_LITERAL)
    {
        uint64_t v = std::stoull(t.sText, nullptr, 16);
        advance();
        return v;
    }
    if (t.sType == TokenType::BIT_LITERAL)
    {
        // strip "0b" prefix
        uint64_t v = std::stoull(t.sText.substr(2), nullptr, 2);
        advance();
        return v;
    }
 
    error("Expected integer literal", t);
}