#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>
#include <vector>
#include "Tokenizer/Tokenizer.hpp"
#include "Parser/Parser.hpp"
#include "CodeGen.hpp"

void printExpr(const node::node* expr)
{
    if (auto* num = dynamic_cast<const node::NumLiteral*>(expr))
    {
        std::cout << "NumLiteral: " << num->value << "\n";
        return;
    }

    if (auto* id = dynamic_cast<const node::Identifier*>(expr))
    {
        std::cout << "Identifier: " << id->name << "\n";
        return;
    }

    if (auto* reg = dynamic_cast<const node::RegRead*>(expr))
    {
        std::cout << "RegRead: " << reg->regFile << "(" << reg->regName << ")\n";
        return;
    }

    if (auto* slice = dynamic_cast<const node::BitSlice*>(expr))
    {
        std::cout << "BitSlice [" << slice->hi->value << " .." << slice->lo->value << "]:\n";
        printExpr(slice->source.get());
        return;
    }

    if (auto* bin = dynamic_cast<const node::BinaryExpr*>(expr))
    {
        std::cout << "BinaryExpr: " << bin->op << "\n";
        std::cout << "LHS:\n";
        printExpr(bin->lhs.get());
        std::cout << "RHS:\n";
        printExpr(bin->rhs.get());
        return;
    }

    if (auto* call = dynamic_cast<const node::BuiltinCall*>(expr))
    {
        std::cout << "BuiltinCall: " << call->name << "\n";
        for (const auto & arg : call->args)
        {
            printExpr(arg.get());
        }
        return;
    }

    if (auto* ifExpr = dynamic_cast<const node::IfExpr*>(expr))
    {
        std::cout << "IfExpr:\n";
        std::cout << "Condition:\n";
        printExpr(ifExpr->condition.get());
        std::cout << "Then:\n";
        printExpr(ifExpr->thenBranch.get());
        std::cout << "Else:\n";
        printExpr(ifExpr->elseBranch.get());
        return;
    }

    std::cout << "UnknownExpr (line " << expr->line << ")\n";
}

void print(const node::node* stmt)
{
    if (auto* let = dynamic_cast<const node::LetBinding*>(stmt))
    {
        std::cout << "LetBinding: " << let->name;
        if (!let->typeName.empty())
        {
            std::cout << " : " << let->typeName;
            if (let->typeParam > 0)
            {
                std::cout << "(" << let->typeParam << ")";
            }
        }
        std::cout << " =\n";
        printExpr(let->init.get());
        return;
    }
}

void printInstruction(const node::InstructionPattern* pattern)
{
    std::cout << "InstructionPattern : " << pattern->mnemonic << "(";
    for (size_t i = 0; i < pattern->operands.size(); ++i)
    {
        if (i > 0)
        {
            std::cout << ", ";
        }
        std::cout << pattern->operands[i];
    }
    std::cout << ")\n";
}

void printClause(const node::FunctionClause* clause)
{
    std::cout << "FunctionClause (line " << clause->line << ")\n";
    std::cout << "Pattern:\n";
    printInstruction(clause->pattern.get());
    std::cout << "Body: \n";
    for(const auto& stmt: clause->body)
    {
        print(stmt.get());
    }
}

void printAST(const node::Program* n)
{
    std::cout << "Program\n";
    for (const auto & clause : n->clauses)
    {
        printClause(clause.get());
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input.sail> [output.hpp]\n";
        std::cerr << "  If output.hpp is omitted, prints AST to stdout.\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open())
    {
        std::cerr << "Error: could not open file " << argv[1] << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string contents = buffer.str();

    // Tokenize
    Tokenizer tokenizer;
    std::vector<Token> tokens = tokenizer.parse(contents);

    // Parse
    Parser parser(tokens);
    auto program = parser.parse();

    if (argc >= 3)
    {
        // get the output path
        std::string outputPath = argv[2];

        CodeGen codegen;
        std::string generated = codegen.generate(program.get());

        std::ofstream out(outputPath);
        if (!out.is_open())
        {
            std::cerr << "Error: could not open output file " << outputPath << "\n";
            return 1;
        }
        out << generated;
        std::cout << "Generated: " << outputPath << "\n";
    }
    else
    {
        // print the ast would be a good idea - need to implement that function - something like :
        //`printNode(program.get());
        printAST(program.get());
    }
    return 0;
}
