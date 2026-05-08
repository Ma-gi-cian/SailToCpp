#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>
#include <vector>
#include "Tokenizer/Tokenizer.hpp"

int main(int argc, char **argv) {

  if (argc <  1) {
    assert(argc > 0);
    std::cerr << "Usage " << argv[0] << " <input.sail>" << std::endl;
    return 1;
  } else  {
    std::stringstream buffer;
    std::ifstream file(argv[1]);
    if(!file.is_open()){
      std::cerr << "Error: opening file " << argv[1] << std::endl;
      return 1;
    }
    buffer << file.rdbuf();

	// we got the data - pass it to the tokenizer 
    std::string contents = buffer.str();

    sail::Tokenizer tokenizer;
    std::vector<sail::Token> tokens = tokenizer.parse(contents);

    for(auto i : tokens)
    {
      std::cout << i.sText << "";
    }

  }
  return 0;
}
