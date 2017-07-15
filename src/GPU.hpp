#pragma once

#include <array>
#include <cstdint>

namespace c8emu {
struct GPU {
  std::array<std::uint8_t, 0x800> data;
  bool canDraw;
};
} // namespace c8emu
