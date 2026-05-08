#include <fstream>
#include <iostream>
#include <sstream>
#include "Tokenizer/Tokenizer.hpp"

int main(int argc, char **argv) {

  if (argc == 1) {
    // print usage
  } else if (argc == 2) {
    std::stringstream buffer;
    std::ifstream file(argv[1]);
    buffer << file.rdbuf();

	// we got the data - pass it to the tokenizer 
    std::string contents = buffer.str();

    sail::Tokenizer tokenizer;
    tokenizer.parse(contents);

  } else {
    // This is tricky - do we want more options,
    std::cerr << "ERROR" << std::endl;
  }
  return 0;
}
