#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <array>

namespace c8emu {
class Screen {
public:
  Screen(std::uint32_t const width, std::uint32_t const height,
         std::array<std::uint8_t, 0x800> const &gfx, std::array<bool, 16> &keys,
         std::uint8_t const scaleFactor);

  inline bool isOpen() const { return m_win.isOpen(); }

  Screen(Screen const &) = delete;
  Screen &operator=(Screen const &) = delete;
  Screen(Screen &&) = delete;
  Screen &operator=(Screen &&) = delete;

  void gpuExec();
  void getInputs();

  void beep();

private:
  std::uint32_t m_width;
  std::uint32_t m_height;
  sf::RenderWindow m_win;
  sf::Texture m_texture;
  sf::Sprite m_sprite;
  std::unique_ptr<sf::Uint8[]> m_pix;
  std::array<std::uint8_t, 0x800> const &m_gfx;
  std::array<bool, 16> &m_keys;
  sf::SoundBuffer m_soundBuff;
  sf::Sound m_beep;

  void loadBeep();
};
} // namespace c8emu
