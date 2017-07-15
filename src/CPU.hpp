#pragma once

#include "GPU.hpp"
#include <array>
#include <cstdint>
#include <functional>

namespace c8emu {
class CPU {
  using byte = std::uint8_t;

public:
  explicit CPU(std::array<byte, 0x1000> &m_memory, GPU &gpu,
               std::array<bool, 16> const &keys);
  void setBeepCallback(std::function<void()> const &beepCallback);
  void execute();

  CPU(CPU const &) = delete;
  CPU &operator=(CPU const &) = delete;
  CPU(CPU &&) = delete;
  CPU &operator=(CPU &&) = delete;

private:
  std::uint16_t m_opcode;
  std::array<byte, 16> m_registers;
  std::uint16_t m_I;
  std::uint16_t m_pc;
  std::array<std::uint16_t, 16> m_stack;
  std::uint16_t m_sp;
  std::array<byte, 0x1000> &m_memory;

  // APU
  byte m_delayTimer;
  byte m_soundTimer;

  // Graphics
  GPU &m_gpu;

  // IO
  std::array<bool, 16> const &m_keys;
  std::function<void()> m_beepCallback;

  // Instructions
  std::array<std::function<void()>, 34> m_instHandler;

  void opcode0();
  void opcode8();
  void opcode13();
  void opcode14();

  void clearScreen();
  void returnFromSubroutine();
  void jumpTo();
  void callSubroutineAt();
  void skipIfEqualNN();
  void skipIfNotEqualNN();
  void skipIfEqualVY();
  void setVxToNN();
  void addNNToVX();
  void skipIfNotEqualVY();
  void setIToNNN();
  void jumpToNNNPlus();
  void setVXRand();
  void setVXToVY();
  void VXorVY();
  void VXandVY();
  void VXxorVY();
  void addVYToVX();
  void subVYFromVX();
  void rshiftVX();
  void setVXToVYSubVX();
  void lshiftVX();
  void drawSpriteVXVY();
  void skipIfVXPressed();
  void skipIfVXNotPressed();
  void setVXToDelayTimer();
  void getKey();
  void setDelayTimer();
  void setSoundTimer();
  void addVXToI();
  void setIToSprite();
  void storeBinVXInI();
  void storeRegistersToMemAtI();
  void fillRegistersWithMemAtI();
};
} // namespace c8emu
