#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>
#include <vector>
#include "Tokenizer/Tokenizer.hpp"
#include "Parser/Parser.hpp"
#include "CodeGen.hpp"
#include "AST/ASTPrettyPrinter.hpp"



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
	ASTPrinter printer;
	printer.print(program.get());
    } return 0;
}
