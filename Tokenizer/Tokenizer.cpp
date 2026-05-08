#include <iostream>
#include "Tokenizer.hpp"

namespace sail {

    using namespace std;

    vector<Token> Tokenizer::parse(const std::string &contents)
    {
        vector<Token> tokens;
        Token current;

        for(int i = 0; i < contents.length(); i++)
        {
            char c = contents.at(i);
            
        }
        std::cout << std::endl;

        return tokens;
    }
}