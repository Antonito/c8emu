#include "Chip8.hpp"
#include <cstddef>
#include <iostream>

int main(int ac, char *av[]) {
  if (ac >= 2) {
    try {
      c8emu::Chip8 chip;
      chip.loadGame(*(av + 1));
      chip.play();
      return EXIT_SUCCESS;
    } catch (std::exception const &e) {
      std::cerr << e.what() << std::endl;
    }
  } else {
    std::cout << "Usage: " << *av << " filename" << std::endl;
  }
  return EXIT_FAILURE;
}
