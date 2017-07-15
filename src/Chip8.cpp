#include "Chip8.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace c8emu {

// Allocating space for constexpr symbols
constexpr std::uint32_t Chip8::screenWidth;
constexpr std::uint32_t Chip8::screenHeight;
constexpr std::array<std::uint8_t, 80> Chip8::fontset;

Chip8::Chip8()
    : m_memory{}, m_gpu{{{0}}, true}, m_cpu(m_memory, m_gpu, m_keys), m_keys{},
      m_screen(screenWidth, screenHeight, m_gpu.data, m_keys, 20) {
  for (std::size_t i = 0; i < 80; ++i)
    m_memory[i] = fontset[i];
  std::srand(static_cast<std::uint32_t>(std::time(nullptr)));
  m_cpu.setBeepCallback([&]() { m_screen.beep(); });
}

void Chip8::loadGame(std::string const &file) {
  // Open file
  std::ifstream input(file, std::ifstream::ate | std::ios::binary);

  if (!input.is_open()) {
    throw std::runtime_error("Cannot open file: " + file);
  }

  // Get size of the file, allocate the buffer and go back to the beginning
  // of the file
  std::size_t len = static_cast<std::size_t>(input.tellg());
  if (m_memory.max_size() - 512 <= len) {
    throw std::runtime_error("Invalid file size"); // TODO: real exception
  }
  std::unique_ptr<byte[]> data = std::make_unique<byte[]>(len);
  input.seekg(0, std::ios::beg);

  // Load file into the buffer
  std::stringstream ss;
  ss << input.rdbuf();
  ss.read(reinterpret_cast<char *>(data.get()),
          static_cast<std::streamsize>(len));

  // Memory map the game
  for (std::size_t i = 0; i < len; ++i) {
    m_memory[i + 512] = data[i];
  }
}

void Chip8::play() {
  m_screen.beep(); // TODO: rm
  while (m_screen.isOpen()) {
    // Single cpu step
    m_cpu.execute();

    // Update drawing when needed
    if (m_gpu.canDraw) {
      m_screen.gpuExec();
    }

    // Capture inputs
    m_screen.getInputs();
  }
}

} // namespace c8emu
