#pragma once
#include <cctype>
#include <string>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include "Parser/AST.hpp"
#include "Parser/CodeGen.hpp"

/**
    This is the code generation module -> we will walk the entire ast - and generate the code.

*/

class CodeGen
{
  public:
    std::string generate(const node::Program* program)
    {
        std::ostringstream out;

        out << "#pragma once\n";
        out << "#include <bit>\n";
        out << "#include <cstdint>\n";

        // Need a mechanism to provide for other include files
        out << "\n";
        out << "namespace pegasus\n{\n";

        std::cout << program->clauses.capacity() << std::endl;

        for (auto & clause : program->clauses)
        {
            emit(out, clause.get());
            clauseNum++;
        }
        out << "}\n";
        return out.str();
    }

  private:
    // (Ma-gi-cian) - TODO: variables are getting redeclared if they are on the left hand side of it
    // - fix it

    /*We need to keep a track of the variables that has been declared already so that it does not */
    int clauseNum = 0;
    std::unordered_map<int, std::vector<std::string>> variables_declared;

    /*A single Program node - can have multiple function clauses and hence we need to  be able to
     * handle them. Hence - we will pass the different function clauses inside of this - and it will
     * output there corresponding functors*/
    void emit(std::ostringstream & out, const node::FunctionClause* clause)
    {
        variables_declared[clauseNum] = {};
        // Get the instruction pattern
        const auto* ext_signature = clause->pattern.get();

        std::string structName = toStructName(ext_signature->mnemonic);

        // Now we need to figure out the operands that are to be used here - comment out the ones
        // that are not being used

        bool Rs1 = false, Rs2 = false, imm = false;
        for (auto & op : ext_signature->operands)
        {
            if (op == "rs1")
                Rs1 = true;

            if (op == "rs2")
                Rs2 = true;

            if (op == "imm")
                imm = true;
        }

        out << "template <typename XLEN> struct " << structName << "\n";
        out << "{\n";
        out << "    XLEN operator()(";
        out << (Rs1 ? "XLEN rs1" : "XLEN /*rs1*/") << ", ";
        out << (Rs2 ? "XLEN rs2" : "XLEN /*rs2*/") << ", ";
        out << (imm ? "uint32_t imm" : "uint32_t /*imm*/");
        out << ") const\n";
        out << "      {\n";

        // All the single seperate lines are now taken as expressions / statements  and we will
        // handle those expressions

        for (auto & stmt : clause->body)
        {
            emitStatement(out, stmt.get(), 3); // Added indentation support
        }

        out << "     }\n";
        out << "   }\n\n";
    }

    /*For toStructName we will convert the signature - to PascalCase + 'Op' like :
        SHA256SIG0  - Sha256sig0Op
    */
    std::string toStructName(const std::string & name)
    {
        std::string result;
        bool found_first = false;

        for (char c : name)
        {
            if (!found_first)
            {
                result += static_cast<char>(toupper(c));
                found_first = true;
            }
            else
            {
                result += c;
            }
        }
        result += "Op";

        return result;
    }

    void emitStatement(std::ostringstream & out, const node::node* stmt, int indent)
    {
        std::string pad(indent * 4, ' ');

        if (auto LetBinding = dynamic_cast<const node::LetBinding*>(stmt))
        {
            std::string ctype = sailTypeToCpp(LetBinding->typeName, LetBinding->typeParam);
            auto it = variables_declared.find(clauseNum);

            bool already_declared =
                (std::find(it->second.begin(), it->second.end(), LetBinding->name)
                 != it->second.end());

            if (!already_declared)
            {
                // Check if this is 'result' - if so, don't make it const
                // because it might be reassigned by X(rd) = ...
                
                // The entire handling of this - should be done at ast level - currently the ast is just plain
                // Its like translating english to assembly directly - no processing on the ast is being done
                bool is_const = (LetBinding->name != "result");

                out << pad;
                if (is_const)
                    out << "const ";
                out << ctype << " " << LetBinding->name << " = ";
                it->second.push_back(LetBinding->name);
            }
            else
            {
                out << pad << LetBinding->name << " = ";
            }

            handleExpr(out, LetBinding->init.get());
            out << ";\n";
            return;
        }

        if (auto* ra = dynamic_cast<const node::RegAssign*>(stmt))
        {
            if (ra->regName == "rd")
            {
                auto it = variables_declared.find(clauseNum);
                auto at = std::find(it->second.begin(), it->second.end(), "result");
                bool is_new = (at == it->second.end());

                // cccccccccheck if the RHS is just an identifier named "result"
                bool is_identity = false;
                if (auto* id = dynamic_cast<const node::Identifier*>(ra->value.get()))
                {
                    if (id->name == "result" && !is_new)
                    {
                        is_identity = true; // It's "result = result", skip it
                    }
                }

                if (!is_identity) // Only emit if it's not a no-op
                {
                    if (is_new)
                    {
                        it->second.push_back("result");
                    }
                    out << pad << (is_new ? "const XLEN " : "") << "result = ";
                    handleExpr(out, ra->value.get());
                    out << ";\n";
                }
            }
            else
            {
                out << pad << "/*" << ra->regFile << "(" << ra->regName << ") = ";
                handleExpr(out, ra->value.get());
                out << ";*/\n";
            }

            return;
        }

        if (auto* rs = dynamic_cast<const node::RetireStmt*>(stmt))
        {
            if (rs->success)
            {
                out << pad << "return result;" << "\n";
            }
            else
            {
                out << pad << "//" << "RETIRE_FAIL" << "\n";
            }
            return;
        }

        if (auto* es = dynamic_cast<const node::ExprStmt*>(stmt))
        {
            out << pad;
            handleExpr(out, es->expr.get());
            out << ";\n";
            return;
        }

        out << pad << "/* <unhandled statement> at line : " << stmt->line << "*/\n";
    }

    void handleExpr(std::ostringstream & out, const node::node* expr)
    {

        if (auto* nl = dynamic_cast<const node::NumLiteral*>(expr))
        {
            out << nl->value;
            return;
        }

        if (auto* id = dynamic_cast<const node::Identifier*>(expr))
        {
            out << id->name;
            return;
        }

        if (auto* register_read = dynamic_cast<const node::RegRead*>(expr))
        {
            out << register_read->regName;
            return;
        }

        if (auto* bc = dynamic_cast<const node::BuiltinCall*>(expr))
        {
            emitBuiltin(out, bc);
            return;
        }

        if (auto* bs = dynamic_cast<const node::BitSlice*>(expr))
        {
            uint64_t hi = bs->hi->value;
            uint64_t lo = bs->lo->value;
            uint64_t width = hi - lo + 1;

            if (width == 32)
            {
                out << "static_cast<uint32_t>(";
                handleExpr(out, bs->source.get());
                out << ")";
            }
            else if (width == 64)
            {
                handleExpr(out, bs->source.get());
            }
            else
            {
                // generate inline bit extraction: (value >> lo) & mask
                // for bits[4..0], generate: (rs2 >> 0) & 0x1F
                // for bits[5..0], generate: (rs2 >> 0) & 0x3F

                uint64_t mask =
                    (1ULL << width) - 1; 

                out << "((";
                handleExpr(out, bs->source.get());
                out << " >> " << lo << ") & 0x" << std::hex << mask << std::dec << ")";
            }
            return;
        }

        if (auto* be = dynamic_cast<const node::BinaryExpr*>(expr))
        {
            out << "(";
            handleExpr(out, be->lhs.get());
            out << " " << sailOpToCpp(be->op) << " ";
            handleExpr(out, be->rhs.get());
            out << ")";
            return;
        }

        if (auto* ue = dynamic_cast<const node::UnaryExpr*>(expr))
        {
            out << sailOpToCpp(ue->op);
            handleExpr(out, ue->operand.get());
            return;
        }

        if (auto* ifExpr = dynamic_cast<const node::IfExpr*>(expr))
        {
            out << "(";
            handleExpr(out, ifExpr->condition.get());
            out << " ? ";
            handleExpr(out, ifExpr->thenBranch.get());
            out << " : ";
            handleExpr(out, ifExpr->elseBranch.get());
            out << ")";
            return;
        }

        out << "/* <unhandled expr> at : " << expr->line << "*/";
    }

    std::string sailOpToCpp(const std::string & op)
    {
        if (op == "@")
            return "|";
        if (op == ">>>")
            return ">>";
        return op;
    }

    /*This handles the function calls - either builtin or from any file should be handled here -
     * TODO: need to provide the compiler for a method to tell where is the file with the function*/
    void emitBuiltin(std::ostringstream & out, const node::BuiltinCall* bc)
    {
        std::string name = bc->name;
        if (name == "ror32" || name == "ror64")
        {
            std::string type = (name == "ror32") ? "uint32_t" : "uint64_t";
            out << "std::rotr<" << type << ">(";
            handleExpr(out, bc->args[0].get());
            out << ", ";
            handleExpr(out, bc->args[1].get());
            out << ")";
            return;
        }

        if (name == "rol32" || name == "rol64")
        {
            std::string type = (name == "rol32") ? "uint32_t" : "uint64_t";
            out << "std::rotl<" << type << ">(";
            handleExpr(out, bc->args[0].get());
            out << ", ";
            handleExpr(out, bc->args[1].get());
            out << ")";
            return;
        }

        if (name == "EXTS")
        {
            out << "static_cast<XLEN>(static_cast<int32_t>(";
            handleExpr(out, bc->args[0].get());
            out << "))";
            return;
        }

        if (name == "EXTZ")
        {
            out << "static_cast<XLEN>(";
            handleExpr(out, bc->args[0].get());
            out << ")";
            return;
        }

        // This is the direct call
        // TODO: Need a method to tell the compiler which particular function is where

        out << name << "(";
        for (size_t i = 0; i < bc->args.size(); ++i)
        {
            if (i > 0)
                out << ", ";
            handleExpr(out, bc->args[i].get());
        }
        out << ")";
    }

    std::string sailTypeToCpp(const std::string & typeName, uint64_t param)
    {
        if (typeName == "bits")
        {
            if (param == 8)
                return "uitn8_t";
            if (param == 16)
                return "uint16_t";
            if (param == 32)
                return "uint32_t";
            if (param == 64)
                return "uint64_t";

            // The fallback - if nothing comes here ( should not be )
            return "uint64_t";
        }

        if (typeName == "bool")
            return "bool";
        if (typeName == "int")
            return "uint64_t";
        if (typeName.empty())
            return "XLEN";
        return typeName;
    }
};