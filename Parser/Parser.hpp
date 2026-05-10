#pragma once
#include "AST.hpp"
#include <stdexcept>
#include <memory>
#include "../Tokenizer/Tokenizer.hpp"

namespace node{
}

struct ParseError : std::runtime_error
{
    Token token;
    ParseError(const std::string & msg, const Token & t)
        : std::runtime_error(msg), token(t) {}
};

class Parser {
    public:
        Parser(std::vector<Token> tokens) : s_tokens(std::move(tokens)), s_pos(0)
        {};

        std::unique_ptr<node::Program> parse();

    private:
        std::vector<Token> s_tokens;
        size_t s_pos;

        const Token & peek(size_t offset=0) const;

        Token advance();

        bool check(TokenType t) const;

        Token expect(TokenType t, const std::string & context);

        bool match(TokenType t);

        bool atEnd() const;


        // These are the grammar rules

        std::unique_ptr<node::FunctionClause> parseFunctionClause();
        std::unique_ptr<node::InstructionPattern> parseInstructionPattern();

        node::NodePtr parseStatement();
        node::NodePtr parseLetBinding();
        node::NodePtr parseRegAssign();

        node::NodePtr parseExpr();
        node::NodePtr parseXorExpr();
        node::NodePtr parseOrExpr();
        node::NodePtr parseAndExpr();
        node::NodePtr parseShiftExpr();
        node::NodePtr parseAddExpr();
        node::NodePtr parseMulExpr();
        node::NodePtr parseUnaryExpr();
        node::NodePtr parsePostfixExpr(); // this one is for handling the bit slicing [31..0]
        node::NodePtr parsePrimaryExpr();
        node::NodePtr parseIfExpr();
        node::NodePtr parseComparisonExpr();

        node::NodePtr parseBuiltinCall(const std::string & name, size_t line);
        node::NodePtr parseRegReadOrSlice(const std::string & regFile);

        uint64_t parseNumLiteralValue();

        // Error helper 
        [[noreturn]] void error(const std::string & msg, const Token &t);
};