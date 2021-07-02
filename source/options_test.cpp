#include <iostream>
#include <string>

int main(int argc, char **argv) {
  for (int i = 1 ; i < argc; i++) {
    std::cout << i << " " << std::stoi(std::string(argv[i])) << "\n";
  }


}