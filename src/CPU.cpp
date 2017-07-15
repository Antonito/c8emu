#include "CPU.hpp"
#include <iostream>

namespace c8emu {
CPU::CPU(std::array<byte, 0x1000> &memory, GPU &gpu,
         std::array<bool, 16> const &keys)
    : m_opcode(0), m_registers{{0}}, m_I(0), m_pc(0x200), m_stack{{0}}, m_sp(0),
      m_memory(memory), m_delayTimer(0), m_soundTimer(0), m_gpu(gpu),
      m_keys(keys), m_beepCallback(),
      m_instHandler{
          {[this]() { this->opcode0(); }, [this]() { this->jumpTo(); },
           [this]() { this->callSubroutineAt(); },
           [this]() { this->skipIfEqualNN(); },
           [this]() { this->skipIfNotEqualNN(); },
           [this]() { this->skipIfEqualVY(); }, [this]() { this->setVxToNN(); },
           [this]() { this->addNNToVX(); }, [this]() { this->opcode8(); },
           [this]() { this->skipIfNotEqualVY(); },
           [this]() { this->setIToNNN(); }, [this]() { this->jumpToNNNPlus(); },
           [this]() { this->setVXRand(); },
           [this]() { this->drawSpriteVXVY(); }, [this]() { this->opcode13(); },
           [this]() { this->opcode14(); }}} {}

void CPU::setBeepCallback(std::function<void()> const &beepCallback) {
  m_beepCallback = beepCallback;
}

void CPU::execute() {
  m_opcode =
      static_cast<std::uint16_t>((m_memory[m_pc] << 8) | m_memory[m_pc + 1]);

  // Treat instruction
  byte const opcodeCmp = (m_opcode & 0xF000) / 0x1000;
  m_instHandler[opcodeCmp]();

  // Update timers
  if (m_delayTimer > 0) {
    --m_delayTimer;
  }

  if (m_soundTimer > 0) {
    if (m_soundTimer == 1) {
      m_beepCallback();
    }
    --m_soundTimer;
  }
}

void CPU::opcode0() {
  switch (m_opcode & 0x000F) {
  case 0x0000: // 0x00E0: Clears the screen
    clearScreen();
    break;

  case 0x000E: // 0x00EE: Returns from subroutine
    returnFromSubroutine();
    break;
  default:
    throw std::runtime_error("Unknown opcode.");
  }
}
void CPU::opcode8() {
  switch (m_opcode & 0x000F) {
  case 0x0000: // 0x8XY0: Sets VX to the value of VY
    setVXToVY();
    break;

  case 0x0001: // 0x8XY1: Sets VX to "VX OR VY"
    VXorVY();
    break;

  case 0x0002: // 0x8XY2: Sets VX to "VX AND VY"
    VXandVY();
    break;

  case 0x0003: // 0x8XY3: Sets VX to "VX XOR VY"
    VXxorVY();
    break;

  case 0x0004: // 0x8XY4: Adds VY to VX. VF is set to 1 when there's
               // a carry, and to 0 when there isn't
    addVYToVX();
    break;

  case 0x0005: // 0x8XY5: VY is subtracted from VX. VF is set to 0
               // when there's a borrow, and 1 when there isn't
    subVYFromVX();
    break;

  case 0x0006: // 0x8XY6: Shifts VX right by one. VF is set to the
               // value of the least significant bit of VX before the
               // shift
    rshiftVX();
    break;

  case 0x0007: // 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when
               // there's a borrow, and 1 when there isn't
    setVXToVYSubVX();
    break;

  case 0x000E: // 0x8XYE: Shifts VX left by one. VF is set to the
               // value of the most significant bit of VX before the
               // shift
    lshiftVX();
    break;

  default:
    throw std::runtime_error("Unknown opcode");
  }
}
void CPU::opcode13() {
  switch (m_opcode & 0x00FF) {
  case 0x009E: // EX9E: Skips the next instruction if the key stored
               // in VX is pressed
    skipIfVXPressed();
    break;

  case 0x00A1: // EXA1: Skips the next instruction if the key stored
               // in VX isn't pressed
    skipIfVXNotPressed();
    break;

  default:
    throw std::runtime_error("Unknown opcode");
  }
}
void CPU::opcode14() {
  switch (m_opcode & 0x00FF) {
  case 0x0007: // FX07: Sets VX to the value of the delay timer
    setVXToDelayTimer();
    break;

  case 0x000A: // FX0A: A key press is awaited, and then stored in VX
    getKey();
    break;

  case 0x0015: // FX15: Sets the delay timer to VX
    setDelayTimer();
    break;

  case 0x0018: // FX18: Sets the sound timer to VX
    setSoundTimer();
    break;

  case 0x001E: // FX1E: Adds VX to I
    addVXToI();
    break;

  case 0x0029: // FX29: Sets I to the location of the sprite for the
               // character in VX. Characters 0-F (in hexadecimal)
               // are represented by a 4x5 font
    setIToSprite();
    break;

  case 0x0033: // FX33: Stores the Binary-coded decimal
               // representation of VX at the addresses I, I plus 1,
               // and I plus 2
    storeBinVXInI();
    break;

  case 0x0055: // FX55: Stores V0 to VX in m_memory starting at
               // address I
    storeRegistersToMemAtI();
    break;

  case 0x0065: // FX65: Fills V0 to VX with values from m_memory
               // starting at address I
    fillRegistersWithMemAtI();
    break;

  default:
    throw std::runtime_error("Unknown opcode.");
  }
}

void CPU::clearScreen() {
  m_gpu.data.fill(0);
  m_gpu.canDraw = true;
  m_pc += 2;
}

void CPU::returnFromSubroutine() {
  --m_sp;
  m_pc = m_stack[m_sp] + 2;
}

void CPU::jumpTo() { m_pc = m_opcode & 0x0FFF; }

void CPU::callSubroutineAt() {
  m_stack[m_sp] = m_pc;
  ++m_sp;
  m_pc = m_opcode & 0x0FFF;
}

void CPU::skipIfEqualNN() {
  m_pc += 2;
  if (m_registers[(m_opcode & 0x0F00) >> 8] == (m_opcode & 0x00FF)) {
    m_pc += 2;
  }
}
void CPU::skipIfNotEqualNN() {
  m_pc += 2;
  if (m_registers[(m_opcode & 0x0F00) >> 8] != (m_opcode & 0x00FF)) {
    m_pc += 2;
  }
}

void CPU::skipIfEqualVY() {
  m_pc += 2;
  if (m_registers[(m_opcode & 0x0F00) >> 8] ==
      m_registers[(m_opcode & 0x00F0) >> 4]) {
    m_pc += 2;
  }
}

void CPU::setVxToNN() {
  m_registers[(m_opcode & 0x0F00) >> 8] = m_opcode & 0x00FF;
  m_pc += 2;
}

void CPU::addNNToVX() {
  m_registers[(m_opcode & 0x0F00) >> 8] += m_opcode & 0x00FF;
  m_pc += 2;
}

void CPU::skipIfNotEqualVY() {
  m_pc += 2;
  if (m_registers[(m_opcode & 0x0F00) >> 8] !=
      m_registers[(m_opcode & 0x00F0) >> 4]) {
    m_pc += 2;
  }
}

void CPU::setIToNNN() {
  m_I = m_opcode & 0x0FFF;
  m_pc += 2;
}

void CPU::jumpToNNNPlus() { m_pc = (m_opcode & 0x0FFF) + m_registers[0]; }

void CPU::setVXRand() {
  m_registers[(m_opcode & 0x0F00) >> 8] =
      (std::rand() % 0xFF) & (m_opcode & 0x00FF);
  m_pc += 2;
}

void CPU::setVXToVY() {
  m_registers[(m_opcode & 0x0F00) >> 8] = m_registers[(m_opcode & 0x00F0) >> 4];
  m_pc += 2;
}

void CPU::VXorVY() {
  m_registers[(m_opcode & 0x0F00) >> 8] |=
      m_registers[(m_opcode & 0x00F0) >> 4];
  m_pc += 2;
}

void CPU::VXandVY() {
  m_registers[(m_opcode & 0x0F00) >> 8] &=
      m_registers[(m_opcode & 0x00F0) >> 4];
  m_pc += 2;
}

void CPU::VXxorVY() {
  m_registers[(m_opcode & 0x0F00) >> 8] ^=
      m_registers[(m_opcode & 0x00F0) >> 4];
  m_pc += 2;
}

void CPU::addVYToVX() {
  if (m_registers[(m_opcode & 0x00F0) >> 4] >
      (0xFF - m_registers[(m_opcode & 0x0F00) >> 8]))
    m_registers[0xF] = 1; // carry
  else
    m_registers[0xF] = 0;
  m_registers[(m_opcode & 0x0F00) >> 8] +=
      m_registers[(m_opcode & 0x00F0) >> 4];
  m_pc += 2;
}

void CPU::subVYFromVX() {
  if (m_registers[(m_opcode & 0x00F0) >> 4] >
      m_registers[(m_opcode & 0x0F00) >> 8])
    m_registers[0xF] = 0; // there is a borrow
  else
    m_registers[0xF] = 1;
  m_registers[(m_opcode & 0x0F00) >> 8] -=
      m_registers[(m_opcode & 0x00F0) >> 4];
  m_pc += 2;
}

void CPU::rshiftVX() {
  m_registers[0xF] = m_registers[(m_opcode & 0x0F00) >> 8] & 0x1;
  m_registers[(m_opcode & 0x0F00) >> 8] >>= 1;
  m_pc += 2;
}

void CPU::setVXToVYSubVX() {
  if (m_registers[(m_opcode & 0x0F00) >> 8] >
      m_registers[(m_opcode & 0x00F0) >> 4]) // VY-VX
    m_registers[0xF] = 0;                    // there is a borrow
  else
    m_registers[0xF] = 1;
  m_registers[(m_opcode & 0x0F00) >> 8] =
      m_registers[(m_opcode & 0x00F0) >> 4] -
      m_registers[(m_opcode & 0x0F00) >> 8];
  m_pc += 2;
}

void CPU::lshiftVX() {
  m_registers[0xF] = m_registers[(m_opcode & 0x0F00) >> 8] >> 7;
  m_registers[(m_opcode & 0x0F00) >> 8] <<= 1;
  m_pc += 2;
}

void CPU::drawSpriteVXVY() {
  std::uint16_t x = m_registers[(m_opcode & 0x0F00) >> 8];
  std::uint16_t y = m_registers[(m_opcode & 0x00F0) >> 4];
  std::uint16_t height = m_opcode & 0x000F;
  std::uint16_t pixel;

  m_registers[0xF] = 0;
  for (std::size_t yline = 0; yline < height; yline++) {
    pixel = m_memory[m_I + yline];
    for (std::size_t xline = 0; xline < 8; xline++) {
      if ((pixel & (0x80 >> xline)) != 0) {
        if (m_gpu.data[(x + xline + ((y + yline) * 64))] == 1) {
          m_registers[0xF] = 1;
        }
        m_gpu.data[x + xline + ((y + yline) * 64)] ^= 1;
      }
    }
  }

  m_gpu.canDraw = true;
  m_pc += 2;
}

void CPU::skipIfVXPressed() {
  m_pc += 2;
  if (m_keys[m_registers[(m_opcode & 0x0F00) >> 8]] != 0) {
    m_pc += 2;
  }
}

void CPU::skipIfVXNotPressed() {
  m_pc += 2;
  if (m_keys[m_registers[(m_opcode & 0x0F00) >> 8]] == 0) {
    m_pc += 2;
  }
}

void CPU::setVXToDelayTimer() {
  m_registers[(m_opcode & 0x0F00) >> 8] = m_delayTimer;
  m_pc += 2;
}

void CPU::getKey() {
  bool keyPress = false;

  for (std::size_t i = 0; i < 16; ++i) {
    if (m_keys[i] != 0) {
      m_registers[(m_opcode & 0x0F00) >> 8] = static_cast<std::uint8_t>(i);
      keyPress = true;
    }
  }

  // If we didn't received a keypress, skip this cycle and try again.
  if (!keyPress)
    return;

  m_pc += 2;
}

void CPU::setDelayTimer() {
  m_delayTimer = m_registers[(m_opcode & 0x0F00) >> 8];
  m_pc += 2;
}

void CPU::setSoundTimer() {
  m_soundTimer = m_registers[(m_opcode & 0x0F00) >> 8];
  m_pc += 2;
}

void CPU::addVXToI() {
  // VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
  if (m_I + m_registers[(m_opcode & 0x0F00) >> 8] > 0xFFF) {
    m_registers[0xF] = 1;
  } else {
    m_registers[0xF] = 0;
  }
  m_I += m_registers[(m_opcode & 0x0F00) >> 8];
  m_pc += 2;
}

void CPU::setIToSprite() {
  m_I = m_registers[(m_opcode & 0x0F00) >> 8] * 0x5;
  m_pc += 2;
}

void CPU::storeBinVXInI() {
  m_memory[m_I] = m_registers[(m_opcode & 0x0F00) >> 8] / 100;
  m_memory[m_I + 1] = (m_registers[(m_opcode & 0x0F00) >> 8] / 10) % 10;
  m_memory[m_I + 2] = (m_registers[(m_opcode & 0x0F00) >> 8] % 100) % 10;
  m_pc += 2;
}

void CPU::storeRegistersToMemAtI() {
  for (std::size_t i = 0; i <= ((m_opcode & 0x0F00) >> 8); ++i) {
    m_memory[m_I + i] = m_registers[i];
  }

  // On the original interpreter, when the operation is done, I = I + X +
  // 1.
  m_I += ((m_opcode & 0x0F00) >> 8) + 1;
  m_pc += 2;
}

void CPU::fillRegistersWithMemAtI() {
  for (std::size_t i = 0; i <= ((m_opcode & 0x0F00) >> 8); ++i) {
    m_registers[i] = m_memory[m_I + i];
  }

  // On the original interpreter, when the operation is done, I = I + X +
  // 1.
  m_I += ((m_opcode & 0x0F00) >> 8) + 1;
  m_pc += 2;
}

} // namespace c8emu
