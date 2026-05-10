#pragma once
#include "../Parser/AST.hpp"
#include <iostream>
#include <string>

class ASTPrinter
{
private:
    int indent_level = 0;
    
    void print_indent() {
        for (int i = 0; i < indent_level; ++i) {
            std::cout << "  ";
        }
    }
    
    void indent() { indent_level++; }
    void dedent() { indent_level--; }

public:
    void print(const node::Program* program) {
        std::cout << "Program\n";
        indent();
        for (const auto& clause : program->clauses) {
            print(clause.get());
        }
        dedent();
    }
    
    void print(const node::FunctionClause* clause) {
        print_indent();
        std::cout << "FunctionClause (line " << clause->line << ")\n";
        indent();
        
        print_indent();
        std::cout << "Pattern:\n";
        indent();
        print(clause->pattern.get());
        dedent();
        
        print_indent();
        std::cout << "Body:\n";
        indent();
        for (const auto& stmt : clause->body) {
            print(stmt.get());
        }
        dedent();
        
        dedent();
    }
    
    void print(const node::InstructionPattern* pattern) {
        print_indent();
        std::cout << "InstructionPattern: " << pattern->mnemonic << "(";
        for (size_t i = 0; i < pattern->operands.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << pattern->operands[i];
        }
        std::cout << ")\n";
    }
    
    void print(const node::node* stmt) {
        if (auto* let = dynamic_cast<const node::LetBinding*>(stmt)) {
            print_indent();
            std::cout << "LetBinding: " << let->name;
            if (!let->typeName.empty()) {
                std::cout << " : " << let->typeName;
                if (let->typeParam > 0) {
                    std::cout << "(" << let->typeParam << ")";
                }
            }
            std::cout << " =\n";
            indent();
            printExpr(let->init.get());
            dedent();
            return;
        }
        
        if (auto* reg = dynamic_cast<const node::RegAssign*>(stmt)) {
            print_indent();
            std::cout << "RegAssign: " << reg->regFile << "(" << reg->regName << ") =\n";
            indent();
            printExpr(reg->value.get());
            dedent();
            return;
        }
        
        if (auto* ret = dynamic_cast<const node::RetireStmt*>(stmt)) {
            print_indent();
            std::cout << "RetireStmt: " << (ret->success ? "SUCCESS" : "FAIL") << "\n";
            return;
        }
        
        if (auto* expr = dynamic_cast<const node::ExprStmt*>(stmt)) {
            print_indent();
            std::cout << "ExprStmt:\n";
            indent();
            printExpr(expr->expr.get());
            dedent();
            return;
        }
        
        print_indent();
        std::cout << "UnknownStmt (line " << stmt->line << ")\n";
    }
    
    void printExpr(const node::node* expr) {
        if (auto* num = dynamic_cast<const node::NumLiteral*>(expr)) {
            print_indent();
            std::cout << "NumLiteral: " << num->value << "\n";
            return;
        }
        
        if (auto* id = dynamic_cast<const node::Identifier*>(expr)) {
            print_indent();
            std::cout << "Identifier: " << id->name << "\n";
            return;
        }
        
        if (auto* reg = dynamic_cast<const node::RegRead*>(expr)) {
            print_indent();
            std::cout << "RegRead: " << reg->regFile << "(" << reg->regName << ")\n";
            return;
        }
        
        if (auto* slice = dynamic_cast<const node::BitSlice*>(expr)) {
            print_indent();
            std::cout << "BitSlice [" << slice->hi->value << ".." << slice->lo->value << "]:\n";
            indent();
            printExpr(slice->source.get());
            dedent();
            return;
        }
        
        if (auto* bin = dynamic_cast<const node::BinaryExpr*>(expr)) {
            print_indent();
            std::cout << "BinaryExpr: " << bin->op << "\n";
            indent();
            print_indent();
            std::cout << "LHS:\n";
            indent();
            printExpr(bin->lhs.get());
            dedent();
            print_indent();
            std::cout << "RHS:\n";
            indent();
            printExpr(bin->rhs.get());
            dedent();
            dedent();
            return;
        }
        
        if (auto* un = dynamic_cast<const node::UnaryExpr*>(expr)) {
            print_indent();
            std::cout << "UnaryExpr: " << un->op << "\n";
            indent();
            printExpr(un->operand.get());
            dedent();
            return;
        }
        
        if (auto* call = dynamic_cast<const node::BuiltinCall*>(expr)) {
            print_indent();
            std::cout << "BuiltinCall: " << call->name << "\n";
            indent();
            for (const auto& arg : call->args) {
                printExpr(arg.get());
            }
            dedent();
            return;
        }
        
        if (auto* ifExpr = dynamic_cast<const node::IfExpr*>(expr)) {
            print_indent();
            std::cout << "IfExpr:\n";
            indent();
            print_indent();
            std::cout << "Condition:\n";
            indent();
            printExpr(ifExpr->condition.get());
            dedent();
            print_indent();
            std::cout << "Then:\n";
            indent();
            printExpr(ifExpr->thenBranch.get());
            dedent();
            print_indent();
            std::cout << "Else:\n";
            indent();
            printExpr(ifExpr->elseBranch.get());
            dedent();
            dedent();
            return;
        }
        
        print_indent();
        std::cout << "UnknownExpr (line " << expr->line << ")\n";
    }
};
