#pragma once
#include "../Parser/AST.hpp"
#include <iostream>
#include <string>
#include <vector>

class ASTPrinter
{
private:
    std::vector<bool> last_child_stack;

    void print_prefix(bool is_last) {
        for (size_t i = 0; i + 1 < last_child_stack.size(); ++i) {
            std::cout << (last_child_stack[i] ? "    " : "│   ");
        }
        std::cout << (is_last ? "└── " : "├── ");
    }

    void push(bool is_last) { last_child_stack.push_back(is_last); }
    void pop()              { last_child_stack.pop_back(); }

public:
    void print(const node::Program* program) {
        std::cout << "Program\n";
        for (size_t i = 0; i < program->clauses.size(); ++i) {
            bool last = (i + 1 == program->clauses.size());
            print_clause(program->clauses[i].get(), last);
        }
    }

private:
    void print_clause(const node::FunctionClause* clause, bool is_last) {
        print_prefix(is_last);
        std::cout << "FunctionClause  (line " << clause->line << ")\n";
        push(is_last);

        // Pattern
        bool body_empty = clause->body.empty();
        print_prefix(false);
        std::cout << "Pattern\n";
        push(false);
        print_pattern(clause->pattern.get(), true);
        pop();

        // Body
        print_prefix(true);
        std::cout << "Body\n";
        push(true);
        for (size_t i = 0; i < clause->body.size(); ++i) {
            bool last_stmt = (i + 1 == clause->body.size());
            print_stmt(clause->body[i].get(), last_stmt);
        }
        pop();

        pop();
    }

    void print_pattern(const node::InstructionPattern* pattern, bool is_last) {
        print_prefix(is_last);
        std::cout << "InstructionPattern  " << pattern->mnemonic << "(";
        for (size_t i = 0; i < pattern->operands.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << pattern->operands[i];
        }
        std::cout << ")\n";
    }

    void print_stmt(const node::node* stmt, bool is_last) {
        if (auto* let = dynamic_cast<const node::LetBinding*>(stmt)) {
            print_prefix(is_last);
            std::cout << "LetBinding  " << let->name;
            if (!let->typeName.empty()) {
                std::cout << " : " << let->typeName;
                if (let->typeParam > 0)
                    std::cout << "(" << let->typeParam << ")";
            }
            std::cout << "\n";
            push(is_last);
            print_expr(let->init.get(), true);
            pop();
            return;
        }

        if (auto* reg = dynamic_cast<const node::RegAssign*>(stmt)) {
            print_prefix(is_last);
            std::cout << "RegAssign  " << reg->regFile << "[" << reg->regName << "]\n";
            push(is_last);
            print_expr(reg->value.get(), true);
            pop();
            return;
        }

        if (auto* ret = dynamic_cast<const node::RetireStmt*>(stmt)) {
            print_prefix(is_last);
            std::cout << "RetireStmt  " << (ret->success ? "SUCCESS" : "FAIL") << "\n";
            return;
        }

        if (auto* expr = dynamic_cast<const node::ExprStmt*>(stmt)) {
            print_prefix(is_last);
            std::cout << "ExprStmt\n";
            push(is_last);
            print_expr(expr->expr.get(), true);
            pop();
            return;
        }

        print_prefix(is_last);
        std::cout << "UnknownStmt  (line " << stmt->line << ")\n";
    }

    void print_expr(const node::node* expr, bool is_last) {
        if (auto* num = dynamic_cast<const node::NumLiteral*>(expr)) {
            print_prefix(is_last);
            std::cout << "NumLiteral  " << num->value << "\n";
            return;
        }

        if (auto* id = dynamic_cast<const node::Identifier*>(expr)) {
            print_prefix(is_last);
            std::cout << "Identifier  " << id->name << "\n";
            return;
        }

        if (auto* reg = dynamic_cast<const node::RegRead*>(expr)) {
            print_prefix(is_last);
            std::cout << "RegRead  " << reg->regFile << "[" << reg->regName << "]\n";
            return;
        }

        if (auto* slice = dynamic_cast<const node::BitSlice*>(expr)) {
            print_prefix(is_last);
            std::cout << "BitSlice  [" << slice->hi->value << ".." << slice->lo->value << "]\n";
            push(is_last);
            print_expr(slice->source.get(), true);
            pop();
            return;
        }

        if (auto* bin = dynamic_cast<const node::BinaryExpr*>(expr)) {
            print_prefix(is_last);
            std::cout << "BinaryExpr  " << bin->op << "\n";
            push(is_last);
            print_expr(bin->lhs.get(), false);
            print_expr(bin->rhs.get(), true);
            pop();
            return;
        }

        if (auto* un = dynamic_cast<const node::UnaryExpr*>(expr)) {
            print_prefix(is_last);
            std::cout << "UnaryExpr  " << un->op << "\n";
            push(is_last);
            print_expr(un->operand.get(), true);
            pop();
            return;
        }

        if (auto* call = dynamic_cast<const node::BuiltinCall*>(expr)) {
            print_prefix(is_last);
            std::cout << "BuiltinCall  " << call->name << "\n";
            push(is_last);
            for (size_t i = 0; i < call->args.size(); ++i) {
                bool last_arg = (i + 1 == call->args.size());
                print_expr(call->args[i].get(), last_arg);
            }
            pop();
            return;
        }

        if (auto* ifExpr = dynamic_cast<const node::IfExpr*>(expr)) {
            print_prefix(is_last);
            std::cout << "IfExpr\n";
            push(is_last);
            print_expr(ifExpr->condition.get(),  false);
            print_expr(ifExpr->thenBranch.get(), false);
            print_expr(ifExpr->elseBranch.get(), true);
            pop();
            return;
        }

        print_prefix(is_last);
        std::cout << "UnknownExpr  (line " << expr->line << ")\n";
    }
};
