#pragma once
#include <string>
#include <vector>
#include <memory>

namespace node
{
    struct node
    {
        size_t line = 0;
        virtual ~node() = default;
    };

    using NodePtr = std::unique_ptr<node>;

    // the numbers - 7, 0x3542, 0b010110
    struct NumLiteral : node
    {
        uint64_t value = 0;
    };

    // rs1, rd, inb, result
    struct Identifier : node
    {
        std::string name;
    };

    struct RegRead : node
    {
        std::string regFile; // if 'X' or 'F' - integer or float registers
        std::string regName; // Name of the register ( rs1, rs2, rnum)
    };

    struct BitSlice : node
    {
        std::unique_ptr<RegRead> source;
        std::unique_ptr<NumLiteral> hi;
        std::unique_ptr<NumLiteral> lo;
    };

    struct BuiltinCall : node
    {
        std::string name;
        std::vector<NodePtr> args;
    };

    struct IfExpr : public node
    {
        NodePtr condition;
        NodePtr thenBranch;
        NodePtr elseBranch;
    };

    struct BinaryExpr : node
    {
        std::string op; // +, -, %,....
        NodePtr lhs;
        NodePtr rhs;
    };

    struct UnaryExpr : node
    {
        std::string op;
        NodePtr operand;
    };

    struct LetBinding : node
    {
        std::string name;
        std::string typeName;
        uint64_t typeParam = 0;
        NodePtr init;
    };

    struct RegAssign : node
    {
        std::string regFile;
        std::string regName;
        NodePtr value;
    };

    struct RetireStmt : node
    {
        bool success = true;
    };

    struct ExprStmt : node
    {
        NodePtr expr;
    };

    struct InstructionPattern : node
    {
        std::string mnemonic;
        std::vector<std::string> operands;
    };

    struct FunctionClause : node
    {
        std::unique_ptr<InstructionPattern> pattern;
        std::vector<NodePtr> body;
    };

    // Root of the file
    struct Program : node
    {
        std::vector<std::unique_ptr<FunctionClause>> clauses;
    };
} // namespace node